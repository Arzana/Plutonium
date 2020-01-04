#pragma once
#include "WinSock.h"

namespace Pu
{
	/* Defines the possible address families for networks. */
	enum class AddressFamily
	{
#ifdef _WIN32
		/* Defines an unspecified network. */
		Unknown = AF_UNSPEC,
		/* Defined local to host (pipes and portals). */
		Unix = AF_UNIX,
		/* Internetwork version 4. */
		IPv4 = AF_INET,
		/* Arpanet IMP addresses. */
		Imp = AF_IMPLINK,
		/* Pup protocols. */
		Pup = AF_PUP,
		/* MIT CHAOS protocols. */
		Chaos = AF_CHAOS,
		/* XEROX NS protocols. */
		NS = AF_NS,
		/* IPX protocols. */
		IPX = AF_IPX,
		/* ISO protocols. */
		ISO = AF_ISO,
		/* European Computer Manufacturers. */
		ECMA = AF_ECMA,
		/* Datakit protocols. */
		DataKit = AF_DATAKIT,
		/* CCITT protocols. */
		CCITT = AF_CCITT,
		/* IBM SNA. */
		SNA = AF_SNA,
		/* DECnet. */
		DECnet = AF_DECnet,
		/* Direct data Link Interface. */
		DLI = AF_DLI,
		/* LAT. */
		LAT = AF_LAT,
		/* NSC Hyperchannel. */
		Hyperchannel = AF_HYLINK,
		/* ApplyTalk. */
		AppleTalk = AF_APPLETALK,
		/* NetBios-style addresses. */
		NetBios = AF_NETBIOS,
		/* VoiceView. */
		VoiceView = AF_VOICEVIEW,
		/* Protocols from Firefox. */
		Fireox = AF_FIREFOX,
		/* Banyan. */
		Banyan = AF_BAN,
		/* Native ATM services. */
		ATM = AF_ATM,
		/* Internetwork version 6. */
		IPv6 = AF_INET6,
		/* Microsoft Wolfpack. */
		Cluster = AF_CLUSTER,
		/* IEEE 1284.4 WG AF. */
		IEEE = AF_12844,
		/* IrDA. */
		IrDA = AF_IRDA,
		/* Network Designers OSI & gateway. */
		NetDesign = AF_NETDES
#endif
	};
}