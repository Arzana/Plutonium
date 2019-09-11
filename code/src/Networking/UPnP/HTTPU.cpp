#include "Networking/UPnP/HTTPU.h"
#include "Core/Diagnostics/Logging.h"

Pu::HttpuRequest::HttpuRequest(const string & uri)
	: method("GET"), port(80)
{
	/* Get the end of the protocol (http). */
	size_t protocolEnd = uri.find_first_of(':');
	protocolEnd = protocolEnd == string::npos ? 0 : protocolEnd + 3;

	/* Get the end of the host and the start of the path. */
	const size_t portStart = uri.find_first_of(':', protocolEnd);
	const size_t pathStart = uri.find_first_of('/', protocolEnd);
	const size_t hostEnd = portStart != string::npos ? portStart : (pathStart != string::npos ? pathStart : uri.size());

	/* Set the port by default if it's in the URI. */
	if (portStart != string::npos)
	{
		/* The end of the port is either the start of the path or the end of the URI. */
		const size_t portEnd = pathStart != string::npos ? pathStart : uri.size() - 1;
		port = static_cast<uint16>(atoi(uri.substr(portStart + 1, portEnd - portStart - 1).c_str()));
	}

	/* Get the host and the path. */
	host = uri.substr(protocolEnd, hostEnd - protocolEnd);
	if (pathStart != string::npos) path = uri.substr(pathStart);
}

void Pu::HttpuRequest::SetMethod(HttpMethod newMethod)
{
	switch (newMethod)
	{
	case HttpMethod::Get:
		method = "GET";
		break;
	case HttpMethod::Head:
		method = "HEAD";
		break;
	case HttpMethod::Post:
		method = "POST";
		break;
	case HttpMethod::Put:
		method = "PUT";
		break;
	case HttpMethod::Delete:
		method = "DELETE";
		break;
	case HttpMethod::Connect:
		method = "CONNECT";
		break;
	case HttpMethod::Options:
		method = "OPTIONS";
		break;
	case HttpMethod::Trace:
		method = "TRACE";
		break;
	case HttpMethod::Patch:
		method = "PATCH";
		break;
	}
}

void Pu::HttpuRequest::AddHeader(const string & header, const string & value)
{
	/* Check if the header was already added. */
	for (decltype(headers)::iterator it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == header)
		{
			/* Just log a warning and replace the header. */
			Log::Warning("Header '%ls' is being added multiple times, replacing old header value '%s' with '%s'!", header.c_str(), it->second.c_str(), value.c_str());
			it->second = value;
			return;
		}
	}

	/* Add the new header. */
	headers.emplace_back(std::make_pair(header, value));
}

void Pu::HttpuRequest::Send(Socket & socket) const
{
	/* Get the receivers IP address and send the message. */
	const IPAddress receiver{ host };
	Send(socket, receiver);
}

void Pu::HttpuRequest::Send(Socket & socket, IPAddress address) const
{
	/* Construct the final request. */
	string msg = method + ' ' + path + " HTTP/1.1\r\n";
	msg += "HOST: " + host + ':' + string::from(port) + "\r\n";
	for (const auto &[key, value] : headers) msg += key + ": " + value + "\r\n";
	msg += "\r\n";

	if(socket.GetProtocol() == Protocol::UDP) socket.Send(msg, address, port);
	else if (socket.GetProtocol() == Protocol::TCP)
	{
		socket.Connect(address, port);
		socket.Send(msg);
	}
	else Log::Fatal("Can only send HTTP(U) requests over UDP or TCP!");
}

bool Pu::HttpuResponse::TryGetHeader(const string & name, string & value) const
{
	/* Check if the key is present in the map, if so return it; otherwise return false. */
	decltype(headers)::const_iterator it = headers.find(name);
	if (it != headers.end())
	{
		value = it->second;
		return true;
	}

	return false;
}

Pu::HttpuResponse Pu::HttpuResponse::Receive(Socket & socket, void * buffer, size_t bufferSize)
{
	const size_t packetSize = socket.Receive(buffer, bufferSize);
	if (!packetSize) Log::Fatal("Cannot create HTTP response from empty DGRAM!");

	socket.Disconnect();
	return HttpuResponse(buffer, packetSize);
}

Pu::HttpuResponse Pu::HttpuResponse::Receive(Socket & socket, void * buffer, size_t bufferSize, IPAddress address, uint16 port)
{
	const size_t packetSize = socket.Receive(buffer, bufferSize, address, port);
	if (!packetSize) Log::Fatal("Cannot create HTTP response from empty DGRAM!");

	return HttpuResponse(buffer, packetSize);
}

Pu::HttpuResponse::HttpuResponse(const void * buffer, size_t size)
	: status(0)
{
	const char *dgram = reinterpret_cast<const char*>(buffer);
	string str1, str2;

	/*
	Loop through the entire packet to get the headers and status code.
	Format for responses is:
	HTTP/[version] [code] [code name]
	[header name]: [header value]
	...
	[empty line]
	[body]
	*/
	size_t i, j;
	for (i = 0, j = 1; i < size; i++)
	{
		char c = dgram[i];

		if (c == '\r') continue;
		if (c == '\n')
		{
			/* If it's the first line check for the status code. */
			if (j)
			{
				j = 0;
				/* Attempt to get the status code. */
				if (string(dgram, dgram + i).contains("HTTP/1.1"))
				{
					status = static_cast<uint16>(strtol(dgram + 9, nullptr, 10));
				}
				else
				{
					Log::Error("First line of a HTTP response must be the status code!");
					return;
				}
			}
			else if (!str2.empty())
			{
				/* Add the header to the list, make sure to remove leading and trailing spaces. */
				headers.emplace(str2, str1.trim(" "));
				str2.clear();
			}
			else
			{
				/* We've reached the body so just exit this loop and continue to body copy. */
				++i;
				break;
			}

			str1.clear();
		}
		else if (c == ':' && str2.empty())
		{
			/*
			We've reached the seperator for the key and value.
			So set the second string to the key and clear the first to receive the value.
			*/
			str2 = str1;
			str1.clear();
		}
		else str1 += c;
	}

	/* Copy over the body is we have one. */
	j = size - 1;
	if (i < j) body = string(dgram + i, dgram + j);
}