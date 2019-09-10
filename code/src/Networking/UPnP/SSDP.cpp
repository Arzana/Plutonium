#include "Networking/UPnP/SSDP.h"
#include "Networking/UPnP/HTTPU.h"
#include "Core/Diagnostics/Logging.h"

Pu::SSDP::SSDP(Socket & socket)
	: socket(&socket), timeout(3), broadcastAddress(IPAddress::GetBroadcast())
{
	/* Reserve enough space in the buffer for responses. */
	buffer.resize(0x1000);

	/* Make sure broadcasting is enabled on the socket. */
	bool broadcast;
	if (socket.GetOption(SocketOption::Broadcast, &broadcast))
	{
		if (!broadcast) socket.SetOption(SocketOption::Broadcast, "1");
	}
}

void Pu::SSDP::Discover(const string & serviceType)
{
	/*
	Construct the request message
	See RFC 2616 4.5 and 5.3 for header info
	*/
	HttpuRequest request("239.255.255.250");		// IPv4 multicast address.
	request.SetMethod("M-SEARCH *");
	request.SetPort(1900);							// SSDP port.
	request.AddHeader("ST", serviceType);			// Add the service type that we want to discover on the network (example for UPnP "ST: upnp:rootdevice").
	request.AddHeader("MAN", "\"ssdp:discover\"");	// Add manditory extension for SSDP discover.
	request.AddHeader("MX", string::from(timeout));	// Define the maximum age (in seconds) before the message expires.

	request.Send(*socket, broadcastAddress);
	lastServiceType = serviceType;
}

bool Pu::SSDP::PollResponse(void)
{
	/* Make sure we poll to see if we have any messages. */
	if (socket->Poll())
	{
		/* The response should come on port 1900. */
		const HttpuResponse response = HttpuResponse::Receive(*socket, buffer.data(), buffer.size(), broadcastAddress, 1900);
		if (response.GetStatusCode() != 200) return false;

		/* The service type should be equal to the one send by our caller. */
		string value;
		if (response.TryGetHeader("ST", value))
		{
			if (value != lastServiceType) return false;
		}

		/* Gets the UUID and the server location. */
		if (response.TryGetHeader("USN", value)) lastResponse.UUID = value;
		if (response.TryGetHeader("LOCATION", value)) lastResponse.Location = value;

		if (response.TryGetHeader("CACHE-CONTROL", value))
		{
			/* TTL is stored as "CACHE-CONTROL: max-age=120". */
			lastResponse.TTL = atoi(value.substr(value.find_first_of('=') + 1).c_str());
		}

		/* Get the properties of the server. */
		if (response.TryGetHeader("SERVER", value))
		{
			const vector<string> parts = value.split('/');
			if (parts.size() == 5)
			{
				lastResponse.ServerName = parts[0];
				lastResponse.OS = parts[1];
				lastResponse.ServiceVersion = parts[2];
				lastResponse.Product = parts[3];
				lastResponse.ProductVersion = parts[4];
			}
			else Log::Warning("SERVER header in SSDP response is invalid!");
		}

		return true;
	}

	return false;
}