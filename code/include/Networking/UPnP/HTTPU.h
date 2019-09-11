#pragma once
#include <map>
#include "Networking/Core/Socket.h"
#include "HttpMethod.h"

namespace Pu
{
	/* Defines a helper object for creating and sending HTTPU web requests. */
	class HttpuRequest
	{
	public:
		/* Creates a new instance of a HTTPU web request from the specified URI. */
		HttpuRequest(_In_ const string &uri);
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

	/* Defines a helper object for receiving and decoding HTTPU web responses. */
	class HttpuResponse
	{
	public:
		/* Copy constructor. */
		HttpuResponse(_In_ const HttpuResponse &value) = default;
		/* Move constructor. */
		HttpuResponse(_In_ HttpuResponse &&value) = default;

		/* Copy assignment. */
		_Check_return_ HttpuResponse& operator =(_In_ const HttpuResponse &other) = default;
		/* Move assignment. */
		_Check_return_ HttpuResponse& operator =(_In_ HttpuResponse &&other) = default;

		/* Gets the status code of the response. */
		_Check_return_ inline uint16 GetStatusCode(void) const
		{
			return status;
		}

		/* Gets whether the status code was informational (1xx). */
		_Check_return_ inline bool IsInformational(void) const
		{
			return status >= 100 && status < 200;
		}

		/* Gets whether the status code was success (2xx). */
		_Check_return_ inline bool IsSuccess(void) const
		{
			return status >= 200 && status < 300;
		}

		/* Gets whether the status code was a redirecion (3xx). */
		_Check_return_ inline bool IsRedirection(void) const
		{
			return status >= 300 && status < 400;
		}

		/* Gets whether the status code was a client error (4xx). */
		_Check_return_ inline bool IsClientError(void) const
		{
			return status >= 400 && status < 500;
		}

		/* Gets whether the status code was a server error (5xx). */
		_Check_return_ inline bool IsServerError(void) const
		{
			return status >= 500 && status < 600;
		}

		/* Gets the body of the response. */
		_Check_return_ inline const string& GetBody(void) const
		{
			return body;
		}

		/* Gets whether this response has a body. */
		_Check_return_ inline bool HasBody(void) const
		{
			return !body.empty();
		}

		/* Attempts to get the value from a specific header (returns whether the header was present). */
		_Check_return_ bool TryGetHeader(_In_ const string &name, _Out_ string &value) const;
		/* Converts the pending packet to a http response (throws if no packet was available!) */
		_Check_return_ static HttpuResponse Receive(_In_ Socket &socket, _In_ void *buffer, _In_ size_t bufferSize);
		/* Converts the pending packet to a http response (throws if no packet was available!) */
		_Check_return_ static HttpuResponse Receive(_In_ Socket &socket, _In_ void *buffer, _In_ size_t bufferSize, _In_ IPAddress address, _In_ uint16 port);

	private:
		HttpuResponse(const void *buffer, size_t size);

		uint16 status;
		std::map<string, string> headers;
		string body;
	};
}