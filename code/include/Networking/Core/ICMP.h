#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the types of ICMP packets. */
	enum class IcmpType
		: octet
	{
		/* An reply to a previously received echo packet. */
		EchoReply = 0,
		/* An error saying some part of the destination was unreachable. */
		DestinationUnreachable = 3,
		/* Congestion control. */
		SourceQuench = 4,
		/* Some packet was redirected. */
		RedirectMessage = 5,
		/* Request an echo (ping) reply from the destination. */
		EchoRequest = 8,
		RouterAdvertisement = 9,
		/* Defines a routers discovery, selection or solicitation. */
		RouterSolicitation = 10,
		/* An error saying that either the TTL or fragment reassembly time was exceeded. */
		TimeExceeded = 11,
		/* Some parameter in the IP header was incorrect. */
		BadIpHeader = 12,
		Timestamp = 13,
		TimestampReply = 14,
		InformationRequest = 15,
		InformationReply = 16,
		AddressMaskRequest = 17,
		AddressMaskReply = 18,
		Traceroute = 30,
		/* Request an extended echo (xping) reply from the restination. */
		ExtendedEchoRequest = 42,
		/* A reply to a previously received extended echo packet. */
		ExtendedEchoReply = 43,
		/* Reserved as a default value. */
		Reserved = 255
	};

	/* Defines the header for a ICMP packet. */
	struct IcmpHeader
	{
		/* The type of the packet. */
		IcmpType Type;
		/* The underlying code associated with the type. */
		octet Code;
		/* The header checksum (can be zero). */
		uint16 Checksum;

		/* Initializes a new instance of a ICMP header. */
		IcmpHeader(void)
			: Type(IcmpType::Reserved), Code(0), Checksum(0)
		{}

		/* Initializes a new instance of a ICMP header for a specific message. */
		IcmpHeader(_In_ IcmpType type, _In_ octet code)
			: Type(type), Code(code), Checksum(0)
		{}
	};
}