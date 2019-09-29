#pragma once
#include "Core/Math/Constants.h"
#include "Protocol.h"

namespace Pu
{
#pragma pack(push, 1)
	/* Defines the 20 octets of the IP header. */
	struct IpHeader
	{
		/* The Length of the Internet Header. */
		octet IHL : 4;
		/* The version of the IP-packet (4 for IPv4). */
		octet Version : 4;
		/* The differentiated Services Code Point of the IP-packet. */
		octet DSCP : 6;
		/* The Explicit Congestion Notification of the IP-packet. */
		octet ECN : 2;
		/* The total length of the packet. */
		uint16 TotalLength;
		/* The unique identifier for the fragments of a single IP datagram. */
		uint16 Id;
		/* Defines the 3 flags and fragment offset of the IP header. */
		uint16 Flags;
		/* The time to live (in seconds) of the datagram. */
		octet TTL;
		/* The protocol used in the data octets. */
		Protocol Protocol;
		/* The one's complement header checksum. */
		uint16 Checksum;
		/* The source IP-address. */
		uint32 SrcAddress;
		/* The destination IP-address. */
		uint32 DstAddress;

		/* Initializes a new instance of a IP header. */
		IpHeader(void)
			: IHL(5), Version(4), DSCP(0), ECN(0),
			TotalLength(0), Id(0), Flags(0),
			TTL(0), Protocol(Protocol::RAW), Checksum(0),
			SrcAddress(0), DstAddress(0)
		{}

		/* Gets the fragment offset of the IP datagram. */
		_Check_return_ inline uint16 GetFragmentOffset(void) const
		{
			return Flags & 0x1FFF;
		}

		/* Specifies whether the router can fragment the packet. */
		_Check_return_ inline bool DontFragment(void) const
		{
			return Flags & 0x4000;
		}

		/* Whether this packet is a fragmented packet. */
		_Check_return_ inline bool MoreFragments(void) const
		{
			return Flags & 0x2000;
		}

		/* Calculates an IP-style checksum. */
		_Check_return_ static inline uint16 CalculateChecksum(_In_ const void *data, _In_ size_t size)
		{
			const uint16 *words = reinterpret_cast<const uint16*>(data);

			uint64 result = 0;
			for (; size > 1; size -= sizeof(uint16)) result += *words++;
			if (size) result += *reinterpret_cast<const octet*>(words);

			result = (result >> 16) + (result & 0xFFFF);
			result += (result >> 16);
			return static_cast<uint16>(~result);
		}
	};
#pragma pack(pop)
}