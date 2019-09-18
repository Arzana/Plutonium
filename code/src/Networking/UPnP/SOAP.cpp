#include "Networking/UPnP/SOAP.h"
#include "Core/Diagnostics/Logging.h"

using namespace tinyxml2;

/* body (used twice) hides class members, but body is a private field of the base class and can't be accessed anyway. */
#pragma warning(push)
#pragma warning(disable:4458)
Pu::SoapRequest::SoapRequest(const string & uri, const string & action)
	: HttpuRequest(uri)
{
	/* SOAP requests often use post. */
	SetMethod(HttpMethod::Post);

	/* The content type is always XML and an action is always present. */
	AddHeader("CONTENT-TYPE", "text/xml; charset=\"utf-8\"");
	AddHeader("SOAPACTION", action);
}

void Pu::SoapRequest::SetBody(const string & body)
{
	/* Cover the SOAP body in the XML envelop. */
	string envelop = "<?xml version=\"1.0\"?>";
	envelop += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
	envelop += "<s:Body>";
	envelop += body;
	envelop += "</s:Body>";
	envelop += "</s:Envelope>";

	/* Add the content length header and call the internal set body function. */
	AddHeader("CONTENT-LENGTH", string::from(envelop.size()));
	HttpuRequest::SetBody(envelop);
}

Pu::SoapResponse Pu::SoapResponse::Receive(Socket & socket, void * buffer, size_t bufferSize)
{
	return SoapResponse(std::move(HttpuResponse::Receive(socket, buffer, bufferSize)));
}

Pu::SoapResponse::SoapResponse(HttpuResponse && resp)
	: HttpuResponse(std::move(resp)), sbody(nullptr)
{
	const string &body = GetBody();

	/* Pass the size so it doesn't have to call strlen internally. */
	doc.Parse(body.c_str(), body.size());

	/* Handle any errors in the document by just exiting. */
	if (doc.Error())
	{
		Log::Error("Invalid XML document was received for SOAP response!");
		return;
	}

	/* Get the envelop element (and early out if it's not there). */
	XMLElement *envelop = doc.FirstChildElement("s:Envelope");
	if (!envelop)
	{
		Log::Error("Invalid SOAP format was received (no envelop)!");
		return;
	}

	/* Get the soap body element (and error out if it's not there). */
	sbody = envelop->FirstChildElement("s:Body");
	if (!sbody)
	{
		Log::Error("Invalid SOAP format was received (no body)!");
		return;
	}
}
#pragma warning(pop)