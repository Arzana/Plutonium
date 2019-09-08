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
}