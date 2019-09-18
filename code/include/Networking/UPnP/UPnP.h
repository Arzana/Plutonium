#pragma once
#include "Networking/Core/IPAddress.h"
#include "Networking/Core/Protocol.h"

namespace Pu
{
	class Socket;

	/* Defines a helper object for the UPnP protocol. */
	class UPnP
	{
	public:
		/* Initializes a new instance of the UPnP helper class. */
		UPnP(void);
		/* Copy constructor. */
		UPnP(_In_ const UPnP&) = default;
		/* Move constructor. */
		UPnP(_In_ UPnP &&value) = default;

		/* Copy assignment. */
		_Check_return_ UPnP& operator =(_In_ const UPnP&) = default;
		/* Move assignment. */
		_Check_return_ UPnP& operator =(_In_ UPnP &&other) = default;

		/* Attempt to discover a UPnP compatible device on this network. */
		_Check_return_ bool Discover(void);
		/* Get the external IP of the senders computer. */
		_Check_return_ IPAddress GetExternalIP(void);
		/* Adds a port forward rule to the remote UPnP root device. */
		_Check_return_ bool ForwardPort(_In_ uint16 port, _In_ Protocol protocol);
		/* Deletes a specifc port forward rule from the remote UPnP root device. */
		void RemovePortForward(_In_ uint16 port, _In_ Protocol protocol);

		/* Sets the timeout of the Discovery part. */
		inline void SetTimeout(_In_ int64 value)
		{
			timeout = value;
		}

	private:
		string controlUrl;
		int64 timeout;
		string buffer;
		IPAddress extAddr;

		static string GetSOAPAction(const string &function);
		static string GetSOAPForwardBody(uint16 port, const string &protocol, const string &host);
		static string GetSOAPDeleteBody(uint16 port, const string &protocol);
		static int GetUPnPErrorCode(const void *body);

		int ForwardPortInternal(uint16 port, const string &protocol);
		void SendSOAPRequest(Socket &socket, const string &function, const string &parameters);
		bool HandleSSDPResponse(const string &location);
		bool Poll(const Socket &socket) const;
		bool SetServiceURL(const void *cur);
	};
}