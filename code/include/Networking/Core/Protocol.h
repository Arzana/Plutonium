#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the network protcols for a socket. */
	enum class Protocol
		: octet
	{
		/* IPv6 Hop-by-Hop Option/ */
		HopByHop = 0,
		/* Internet Control Message Protocol/ */
		ICMP =  1,
		/* Internet Group Menagement Protocol. */
		IGMP = 2,
		/* Gateway to Gateway Protocol. */
		GGP = 3,
		/* Transmission Control Protocol. */
		TCP = 6,
		/* PARC Universal Packet Protocol. */
		PUP = 12,
		/* User Datagram Protocol. */
		UDP = 17,
		/* Inernet Datagram Protocol. */
		IDP = 22,
		/* Net Disk Protocol. */
		ND = 77,
		/* Raw IP packet protocol. */
		RAW = 255
	};

	/* Converts the specific internet protocol type to a string. */
	_Check_return_ inline const char* to_string(_In_ Protocol protocol)
	{
		switch (protocol)
		{
		case Protocol::ICMP:
			return "ICMP";
		case Protocol::IGMP:
			return "IGMP";
		case Protocol::GGP:
			return "GGP";
		case Protocol::TCP:
			return "TCP";
		case Protocol::PUP:
			return "PUP";
		case Protocol::UDP:
			return "UDP";
		case Protocol::IDP:
			return "IDP";
		case Protocol::ND:
			return "ND";
		case Protocol::RAW:
			return "RAW";
		default:
			return "UNKNOWN";
		}
	}
}