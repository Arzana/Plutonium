#pragma once
#include "HTTPU.h"
#include <tinyxml/tinyxml2.h>

namespace Pu
{
	/* Defines a helper object for creating and sending SOAP web requests. */
	class SoapRequest
		: public HttpuRequest
	{
	public:
		/* Initializes a new instance of a SOAP request for a specific action to a specified URI. */
		SoapRequest(_In_ const string &uri, _In_ const string &action);
		/* Copy constructor. */
		SoapRequest(_In_ const SoapRequest &value) = default;
		/* Move constructor. */
		SoapRequest(_In_ SoapRequest &&value) = default;

		/* Copy assignment. */
		_Check_return_ SoapRequest& operator =(_In_ const SoapRequest &other) = default;
		/* Move assignment. */
		_Check_return_ SoapRequest& operator =(_In_ SoapRequest &&other) = default;

		/* Sets the SOAP body to the specified string. */
		virtual void SetBody(_In_ const string &body);
	};

	/* Defines a helper object for receiving and decoding SOAP web responses. */
	class SoapResponse
		: public HttpuResponse
	{
	public:
		/* Copy constructor. */
		SoapResponse(_In_ const SoapResponse &value) = default;
		/* Move constructor. */
		SoapResponse(_In_ SoapResponse &&value) = default;

		/* Copy assignment. */
		_Check_return_ SoapResponse& operator =(_In_ const SoapResponse &other) = default;
		/* Move assignment. */
		_Check_return_ SoapResponse& operator =(_In_ SoapResponse &&other) = default;

		/* Converts the pending packet to a soap response (throws if no packet was available!) */
		_Check_return_ static SoapResponse Receive(_In_ Socket &socket, _In_ void *buffer, _In_ size_t bufferSize);

		/* Gets the XML element <s:body> from the SOAP response. */
		_Check_return_ inline const tinyxml2::XMLElement& GetSoapBody(void) const
		{
			return *sbody;
		}

	private:
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLElement *sbody;

		SoapResponse(HttpuResponse &&resp);
	};
}