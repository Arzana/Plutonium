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
	/* Receive polls internally so we only have to check if the size is greater than zero. */
	const size_t size = socket->Receive(buffer.data(), buffer.size(), broadcastAddress, 1900);
	if (size)
	{
		/* Divide the HTTP like response into it's parts. */
		const vector<string> headers = buffer.split({ '\r', '\n' });
		for (const string &header : headers)
		{
			/* Split the header into it's key and value. */
			const size_t valueStart = header.find_first_of(':');

			/* Check for the HTTP status code. */
			if (valueStart == string::npos && header.contains("HTTP/1.1"))
			{
				/* "HTTP/1.1 " is 9 characters long and a HTTP status code is always 3 characters long. */
				lastResponse.StatusCode = static_cast<uint16>(atoi(header.substr(9, 3).c_str()));
				continue;
			}

			/* Get the key and the value of the header (+1 to remove the ':' and remove leading spaces). */
			const string key = header.substr(0, valueStart);
			const string value = header.substr(valueStart + 1).trim_front(" ");

			if (key == "ST" && value != lastServiceType)
			{
				/*  The response service type must be equal to the request service type. */
				false;
			}
			else if (key == "USN") lastResponse.UUID = std::move(value);
			else if (key == "SERVER")
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
			else if (key == "LOCATION") lastResponse.Location = value;
			else if (key == "CACHE-CONTROL")
			{
				/* TTL is stored as "CACHE-CONTROL: max-age=120". */
				lastResponse.TTL = atoi(value.substr(value.find_first_of('=') + 1).c_str());
			}
		}

		return true;
	}

	return false;
}