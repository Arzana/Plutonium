#pragma once
#include "Core/Platform/Windows/Windows.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif

namespace Pu
{
	/* Defines the network protcols for a socket. */
	enum class Protocol
	{
#ifdef _WIN32
		/* Internet Control Message Protocol/ */
		ICMP =  IPPROTO_ICMP,
		/* Internet Group Menagement Protocol. */
		IGMP = IPPROTO_IGMP,
		/* Gateway to Gateway Protocol. */
		GGP = IPPROTO_GGP,
		/* Transmission Control Protocol. */
		TCP = IPPROTO_TCP,
		/* PARC Universal Packet Protocol. */
		PUP = IPPROTO_PUP,
		/* User Datagram Protocol. */
		UDP = IPPROTO_UDP,
		/* Inernet Datagram Protocol. */
		IDP = IPPROTO_IDP,
		/* Net Disk Protocol. */
		ND = IPPROTO_ND,
		/* Raw IP packet protocol. */
		RAW = IPPROTO_RAW
#endif
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