#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Defines an easy of use object for creating IP addresses. */
	struct IPAddress
	{
	public:
		/* Creates a new IP address from a big-endian packet value. */
		IPAddress(_In_ uint64 address);
		/* Creates a new IPv4 address from individual period seperated parts.  */
		IPAddress(_In_ octet nw1, _In_ octet nw2, _In_ octet nw3, _In_ octet host);
		/* Creates a new IP address from the specified string. */
		IPAddress(_In_ const string &address);

		/* 0.0.0.0 */
		_Check_return_ static IPAddress GetAny(void);
		/* 255.255.255.255 */
		_Check_return_ static IPAddress GetBroadcast(void);
		/* 127.0.0.1 */
		_Check_return_ static IPAddress GetLoopback(void);

		/* Gets the packet value of the address. */
		_Check_return_ inline uint64 GetAddress(void) const
		{
			return address;
		}

		/* Gets the IP address as a string. */
		_Check_return_ string GetAddressStr(void) const;

	private:
		uint64 address;
	};
}