#include "Networking/UPnP/HTTPU.h"
#include "Core/Diagnostics/Logging.h"

Pu::HttpuRequest::HttpuRequest(const string & url)
	: method("GET"), port(80)
{
	/* Get defining points in the URL. */
	const size_t protocolEnd = url.find_first_of(':');
	const size_t pathStart = url.find_first_of('/');
	const size_t queryStart = url.find_first_of('?');

	/* Get the start and end of the host. */
	const size_t hostStart = protocolEnd == string::npos ? 0 : protocolEnd;
	const size_t hostEnd = pathStart == string::npos ? (queryStart == string::npos ? url.size() : queryStart) : pathStart;

	/* Set the host and the path. */
	host = url.substr(hostStart, hostEnd);
	path = url.substr(hostEnd);
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
	const IPAddress receiver = socket.GetAddress(host, port);
	Send(socket, receiver);
}

void Pu::HttpuRequest::Send(Socket & socket, IPAddress address) const
{
	/* Construct the final request. */
	string msg = method + ' ' + path + " HTTP/1.1\r\n";
	msg += "HOST: " + host + ':' + string::from(port) + "\r\n";
	for (const auto &[key, value] : headers) msg += key + ": " + value + "\r\n";
	msg += "\r\n";

	socket.Send(msg, address, port);
}