#pragma once
#include "Networking/Core/Socket.h"
#include "SSDPResponse.h"

namespace Pu
{
	/* Defines a helper object for the Simple Service Discovery Protocol (SSDP). */
	class SSDP
	{
	public:
		/* Initializes a new instance of a SSDP object with a specific UDP socket (broadcasting will be turned on if not already turned on!). */
		SSDP(_In_ Socket &socket);
		/* Copy constructor. */
		SSDP(_In_ const SSDP &value) = default;
		/* Move constructor. */
		SSDP(_In_ SSDP &&value) = default;

		/* Copy assignment. */
		_Check_return_ SSDP& operator =(_In_ const SSDP &other) = default;
		/* Move assignment. */
		_Check_return_ SSDP& operator =(_In_ SSDP &&other) = default;

		/* Gets the current timeout set for the SSDP request. */
		_Check_return_ inline uint32 GetTimeout(void) const
		{
			return timeout;
		}

		/* Gets the last response polled from the socket. */
		_Check_return_ const SSDPResponse& GetLastResponse(void) const
		{
			return lastResponse;
		}

		/* Sets the timeout for the SSDP requests. */
		inline void SetTimeout(_In_ uint32 value)
		{
			timeout = value;
		}

		/* Broadcasts a discovery message to the entire subnet for the specified service. */
		void Discover(_In_ const string &serviceType);
		/* Check whether a service has responded to the last discovery request and stores that response. */
		_Check_return_ bool PollResponse(void);

	private:
		Socket *socket;
		uint32 timeout;
		IPAddress broadcastAddress;

		string lastServiceType;
		SSDPResponse lastResponse;
		string buffer;
	};
}