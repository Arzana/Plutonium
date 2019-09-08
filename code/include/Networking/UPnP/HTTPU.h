#pragma once
#include "Networking/Core/Socket.h"
#include "HttpMethod.h"

namespace Pu
{
	/* Defines a helper object for creating and sending HTTPU web requests. */
	class HttpuRequest
	{
	public:
		/* Creates a new instance of a HTTPU web request from the specified URL. */
		HttpuRequest(_In_ const string &url);
		/* Copy constructor. */
		HttpuRequest(_In_ const HttpuRequest &value) = default;
		/* Move constructor. */
		HttpuRequest(_In_ HttpuRequest &&value) = default;

		/* Copy assignment. */
		_Check_return_ HttpuRequest& operator =(_In_ const HttpuRequest &other) = default;
		/* Move assignment. */
		_Check_return_ HttpuRequest& operator =(_In_ HttpuRequest &&other) = default;

		/* Sets the port to use for the HTTPU request. */
		inline void SetPort(_In_ uint16 value)
		{
			port = value;
		}

		/* Sets a custom method for the HTTPU request. */
		inline void SetMethod(_In_ const string &newMethod)
		{
			method = newMethod;
		}

		/* Sets the method to use in the request. */
		void SetMethod(_In_ HttpMethod newMethod);
		/* Adds a header to the request. */
		void AddHeader(_In_ const string &header, _In_ const string &value);
		/* Send the request over HTTPU. */
		void Send(_In_ Socket &socket) const;
		/* Send the request over HTTP to a specific endpoint. */
		void Send(_In_ Socket &socket, _In_ IPAddress address) const;

	private:
		string method;
		string host;
		string path;
		uint16 port;
		vector<std::pair<string, string>> headers;
	};
}