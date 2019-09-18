#include "Networking/UPnP/UPnP.h"
#include "Networking/UPnP/SSDP.h"
#include "Networking/UPnP/SOAP.h"
#include "Core/Diagnostics/Stopwatch.h"

using namespace tinyxml2;

/*
These are the error codes that we handle for adding port mappings. 
The others (described in http://upnp.org/specs/gw/UPnP-gw-WANIPConnection-v1-Service.pdf 2.4.16.4)
Can either not occur or just return as a default error.
*/
#define WILDCARD_NOT_PERMITTED_EXT_PORT			716
#define CONFLICT_MAPPING_ENTRY					718
#define REMOTE_HOST_ONLY_SUPPORTS_WILDCARD		726
#define EXTERNAL_PORT_ONLY_SUPPORTS_WILDCARD	727

/* We're using the V1.0 standard instead of the newer V2.0 because most devices do not yet support it. */
constexpr const char *XMLNS = "urn:schemas-upnp-org:device-1-0";
constexpr const char *DEVICE_TYPE = "InternetGatewayDevice";
constexpr const char *SERVICE_TYPE = "urn:schemas-upnp-org:service:WANIPConnection:1";

Pu::UPnP::UPnP(void)
	: timeout(3), extAddr(0)
{
	/* Allocate a response buffer. */
	buffer.resize(0x1000);
}

bool Pu::UPnP::Discover(void)
{
	/* Setup a temporary UDP socket. */
	Socket udp{ AddressFamily::IPv4, SocketType::Dgram, Protocol::UDP };
	SSDP ssdp{ udp };

	/* Broadcast a SSDP discovery message to the subnet. */
	ssdp.Discover("upnp:rootdevice");

	/* Wait for a service to respond. */
	Stopwatch sw = Stopwatch::StartNew();
	do
	{
		if (ssdp.PollResponse())
		{
			sw.End();
			break;
		}
	} while (sw.Seconds() < timeout);

	/* No device responded so just exit. */
	if (sw.Seconds() >= timeout) return false;

	/* Get the SSDP response and log it's discovery. */
	const SSDPResponse rootdevice = ssdp.GetLastResponse();
	Log::Verbose("UPnP rootdevice found on %s!", rootdevice.ServerName.c_str());

	/* Query the root device's description for the service URL and to error check it. */
	return HandleSSDPResponse(rootdevice.Location);
}

Pu::IPAddress Pu::UPnP::GetExternalIP(void)
{
	/* Make sure we don't do another web request but just use a cached value. */
	if (extAddr.GetAddress()) return extAddr;

	Socket tcp{ AddressFamily::IPv4, SocketType::Stream, Protocol::TCP };
	SendSOAPRequest(tcp, "GetExternalIPAddress", "");

	/* Wait for the root device to respond. */
	if (!Poll(tcp)) return IPAddress::GetAny();

	/* Get the XML response. */
	SoapResponse resp = SoapResponse::Receive(tcp, buffer.data(), buffer.size());
	if (resp.GetStatusCode() != 200) return IPAddress::GetAny();

	/* Get the response element. */
	const XMLElement *sresp = resp.GetSoapBody().FirstChildElement("u:GetExternalIPAddressResponse");
	if (!sresp)
	{
		Log::Error("GetExternalIPAddress SOAP response didn't contain a response element!");
		return IPAddress::GetAny();
	}

	/* Get the IP address element. */
	const XMLElement *address = sresp->FirstChildElement("NewExternalIPAddress");
	if (!address)
	{
		Log::Error("GetExternalIpAddress SOAP response didn't contain a new external IP address element!");
		return IPAddress::GetAny();
	}

	/* Return the senders external IP address. */
	return extAddr = IPAddress(address->GetText());
}

bool Pu::UPnP::ForwardPort(uint16 port, Protocol protocol)
{
	/* Check for invalid arguments. */
	if (protocol != Protocol::UDP && protocol != Protocol::TCP)
	{
		Log::Error("Only UDP and TCP ports can be forwarded!");
		return false;
	}

	/* Attempt to forward the port with the parameters specified by the user. */
	const string protStr = to_string(protocol);
	int code = ForwardPortInternal(port, protStr);
	if (code)
	{
		/* 
		Just pick a random port ourselves if wildcards are not supported. 
		Otherwise log the usefull errors (if any) and exit.
		*/
		if (code == WILDCARD_NOT_PERMITTED_EXT_PORT)
		{
			port = static_cast<uint16>(random(0, maxv<uint16>()));
			Log::Warning("UPnP root device doesn't support wildcard port mappings, Plutonium picked random port: %u.", port);
			code = ForwardPortInternal(port, protStr);
		}
		else if (code == CONFLICT_MAPPING_ENTRY) Log::Error("Unable to forward port %u because it's already in use!", port);
		else if (code == REMOTE_HOST_ONLY_SUPPORTS_WILDCARD) Log::Error("Unable to forward port %u to host %s because UPnP root device only supports host wildcards!", port, extAddr.GetAddressStr().c_str());
		else if (code == EXTERNAL_PORT_ONLY_SUPPORTS_WILDCARD) Log::Error("Unable to forward specific port %u because UPnP root device only supports wildcards!", port);
	}

	/* Log the addition of the port rule if it was successful. */
	if (!code) Log::Message("Added %s port rule (%u) to UPnP root device.", protStr.c_str(), port);

	/* A code of zero indicates success. */
	return !code;
}

void Pu::UPnP::RemovePortForward(uint16 port, Protocol protocol)
{
	/* Check for invalid arguments. */
	if (protocol != Protocol::UDP && protocol != Protocol::TCP)
	{
		Log::Error("Only UDP and TCP ports can be forwarded!");
		return;
	}

	/* Send the request to delete the forward rule. */
	Socket tcp{ AddressFamily::IPv4, SocketType::Stream, Protocol::TCP };
	const char *protStr = to_string(protocol);

	SendSOAPRequest(tcp, "DeletePortMapping", GetSOAPDeleteBody(port, protStr));

	/* Wait for the root device to respond. */
	if (!Poll(tcp)) return;

	/* Get the XML response and handle errors. */
	SoapResponse resp = SoapResponse::Receive(tcp, buffer.data(), buffer.size());
	if (resp.GetStatusCode() != 200) Log::Error("Unable to delete forward rule (code: %d)!", GetUPnPErrorCode(&resp.GetSoapBody()));
	else Log::Message("Removed %s port rule (%u) from UPnP root device.", protStr, port);
}

Pu::string Pu::UPnP::GetSOAPAction(const string & function)
{
	string result = "\"";
	result += SERVICE_TYPE;
	result += '#';
	result += function;
	result += '"';
	return result;
}

Pu::string Pu::UPnP::GetSOAPForwardBody(uint16 port, const string & protocol, const string &host)
{
	const string portStr = string::from(port);

	string result = "<NewRemoteHost></NewRemoteHost>";		// Don't force to one remote host.
	result += "<NewExternalPort>" + portStr + "</NewExternalPort>";
	result += "<NewProtocol>" + protocol + "</NewProtocol>";
	result += "<NewInternalPort>" + portStr + "</NewInternalPort>";
	result += "<NewInternalClient>" + host + "</NewInternalClient>";
	result += "<NewEnabled>1</NewEnabled>";					// Automatically enbale the rule.
	result += "<NewPortMappingDescription>Plutonium UPnP rule</NewPortMappingDescription>";
	result += "<NewLeaseDuration>0</NewLeaseDuration>";		// Just keep the lease until we delete it ourselves.
	return result;
}

Pu::string Pu::UPnP::GetSOAPDeleteBody(uint16 port, const string & protocol)
{
	const string portStr = string::from(port);

	string result = "<NewRemoteHost></NewRemoteHost>";		// Don't force to one remote host.
	result += "<NewExternalPort>" + portStr + "</NewExternalPort>";
	result += "<NewProtocol>" + protocol + "</NewProtocol>";
	return result;
}

int Pu::UPnP::GetUPnPErrorCode(const void * body)
{
	const XMLElement &sbody = *reinterpret_cast<const XMLElement*>(body);

	const XMLElement *fault = sbody.FirstChildElement("s:Fault");
	if (fault)
	{
		const XMLElement *detail = fault->FirstChildElement("detail");
		if (detail)
		{
			const XMLElement *error = detail->FirstChildElement("UPnPError");
			if (error)
			{
				const XMLElement *code = error->FirstChildElement("errorCode");
				return code->IntText(-1);
			}
		}
	}

	Log::Error("UPnP root device threw an error but the error code could not be determined!");
	return -1;
}

int Pu::UPnP::ForwardPortInternal(uint16 port, const string & protocol)
{
	/* Create the needed socket and get the local host string. */
	Socket tcp{ AddressFamily::IPv4, SocketType::Stream, Protocol::TCP };
	const string host = tcp.GetHost().GetAddressStr();

	SendSOAPRequest(tcp, "AddPortMapping", GetSOAPForwardBody(port, protocol, host));

	/* Wait for the root device to respond. */
	if (!Poll(tcp)) return -1;

	/* Get the XML response and handle errors. */
	SoapResponse resp = SoapResponse::Receive(tcp, buffer.data(), buffer.size());
	return resp.GetStatusCode() == 200 ? 0 : GetUPnPErrorCode(&resp.GetSoapBody());
}

void Pu::UPnP::SendSOAPRequest(Socket & socket, const string & function, const string & parameters)
{
	/* Add the function element for this request. */
	string body = "<u:";
	body += function;
	body += " xmlns:u=\"";
	body += SERVICE_TYPE;
	body += "\">";

	/* Add aditional parameters inside of element. */
	if (!parameters.empty()) body += parameters;

	/* Close the element. */
	body += "</u:";
	body += function;
	body += '>';

	/* Construct the SOAP request. */
	SoapRequest req{ controlUrl, GetSOAPAction(function) };
	req.SetBody(body);

	/* Send the request over the TCP socket. */
	req.Send(socket);
}

bool Pu::UPnP::HandleSSDPResponse(const string & location)
{
	Socket tcp{ AddressFamily::IPv4, SocketType::Stream, Protocol::TCP };

	/* Send the request for the XML description over the TCP socket. */
	HttpuRequest xmlReq{ location };
	xmlReq.Send(tcp);

	/* Wait a small amount before requesting the response. */
	if (!Poll(tcp)) return false;

	/* Get the response and recreate the socket. */
	HttpuResponse xmlResp = HttpuResponse::Receive(tcp, buffer.data(), buffer.size());
	if (xmlResp.GetStatusCode() != 200) return false;

	/* Parse the resulting XML and make sure it's correct. */
	XMLDocument doc;
	doc.Parse(xmlResp.GetBody().c_str(), xmlResp.GetBody().size());
	if (doc.Error())
	{
		Log::Error("Invalid XML document was received from UPnP root device!");
		return false;
	}

	/* The root element should always have the correct XML namespace. */
	XMLElement *root = doc.FirstChildElement();
	if (string(root->Attribute("xmlns")) != XMLNS)
	{
		Log::Error("Invalid XML namespace used in UPnP root device description, should be '%s'!", XMLNS);
		return false;
	}

	/* A device element should be present that gives information about the device and its services. */
	XMLElement *device = root->FirstChildElement("device");
	if (!device)
	{
		Log::Error("No 'device' element was found in UPnP root device!");
		return false;
	}

	/* The device type should contain InternetGatewayDevice. */
	XMLElement *deviceType = device->FirstChildElement("deviceType");
	if (!deviceType)
	{
		Log::Error("No 'deviceType' element was found in UPnP root device!");
		return false;
	}
	else if (!string(deviceType->GetText()).contains(DEVICE_TYPE))
	{
		Log::Error("UPnP device type is not equal to '%s'!", DEVICE_TYPE);
		return false;
	}

	/* Recursively get the control URL. */
	controlUrl = xmlReq.GetWebHost();
	return SetServiceURL(root);
}

bool Pu::UPnP::Poll(const Socket & socket) const
{
	/* Wait for a small moment before polling the socket. */
	Stopwatch sw = Stopwatch::StartNew();
	do
	{
		if (socket.Poll())
		{
			sw.End();
			break;
		}
	} while (sw.Seconds() < timeout);

	/* This should never occur. */
	if (sw.Seconds() >= timeout)
	{
		Log::Error("Connection was broken between the client and the UPnP root device!");
		return false;
	}

	return true;
}

bool Pu::UPnP::SetServiceURL(const void * cur)
{
	/* Cast is needed because we don't want tinyxml2 to be included in the header. */
	const XMLElement *elem = reinterpret_cast<const XMLElement*>(cur);

	/* Check if this element is a service. */
	if (string(elem->Value()) == "service")
	{
		const XMLElement *type = elem->FirstChildElement("serviceType");
		if (!type)
		{
			Log::Error("Invalid XML, service did not contain a service type from UPnP root device!");
			return false;
		}

		/* Check for the correct service type. */
		if (string(type->GetText()) == SERVICE_TYPE)
		{
			const XMLElement *url = elem->FirstChildElement("controlURL");
			if (!url)
			{
				Log::Error("Invalid XML, service did not contain a control URL from UPnP root device!");
				return false;
			}

			/* Add the path to the host and return a successfull result. */
			controlUrl += url->GetText();
			return true;
		}
	}

	/* Search the children for the URL. */
	for (cur = elem->FirstChildElement(); cur; cur = elem->NextSiblingElement())
	{
		/* Search the children. */
		if (SetServiceURL(cur)) return true;
	}

	/* Check for the next sibling. */
	cur = elem->NextSiblingElement();
	if (cur) return SetServiceURL(cur);

	return false;
}