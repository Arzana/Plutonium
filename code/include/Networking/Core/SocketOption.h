#pragma once
#include "WinSock.h"

namespace Pu
{
	/* Defines all possible settable options for network sockets. */
	enum class SocketOption
	{
#ifdef _WIN32
		/* Configures the socket for sending broadcast data (socket level, boolean). */
		Broadcast = SO_BROADCAST,
		/* Incoming connections are to be accepted or rejected by the application (socket level, boolean). */
		ConditionalAccept = SO_CONDITIONAL_ACCEPT,
		/* Enables or disables debug output (socket level, boolean). */
		Debug = SO_DEBUG,
		/* Doesn't block close waiting for unsent data (socket level, boolean). */
		DontLinger = SO_DONTLINGER,
		/* Whether outgoing data should be sent on the interface the socket is bound to and not routed on some other interface (socket level, boolean). */
		DontRoute = SO_DONTROUTE,
		/* Whether the socket takes exclusive access (socket level, boolean). */
		ExclusiveAddressUse = SO_EXCLUSIVEADDRUSE,
		/* Enables or disables sending keep-alive packets for a socket connection (socket level, boolean). */
		KeepAlive = SO_KEEPALIVE,
		/* Lingers on close if unsent data is present (socket level, LINGER). */
		Linger = SO_LINGER,
		/* Disables the Nagle algorith for send coalescing (TCP level, boolean). */
		NoDelay = TCP_NODELAY,
		/* Out of bounds data should be returned in-line with regular data (socket level, boolean). */
		OutOfBandInline = SO_OOBINLINE,
		/* Specifies the total per-socket buffer space reserved for receives (socket level, int). */
		ReceiveBuffer = SO_RCVBUF,
		/* Specifies the timeout (in milliseconds) for blocking receive calls (socket level, DWORD). */
		ReceiveTimeout = SO_RCVTIMEO,
		/* Allows the socket to be bounds to an address that's already in use (socket level, boolean). */
		ReuseAddress = SO_REUSEADDR,
		/* Specified the total per-socket buffer space reserved for sends (socket level, int). */
		SendBuffer = SO_SNDBUF,
		/* Specifies the timeout (in milliseconds) for blocking send calls (socket level, DWORD). */
		SendTimeout = SO_SNDTIMEO,
		/* Specified that packets through this socket are not allowed to be fragmented (IP level, boolean). */
		DontFragment = IP_DONTFRAGMENT,
		/* Gets the system's estimate of the path MTU (IP level, DWORD). */
		MTU = IP_MTU,
		/* Specify that packets through this socket will only be used for Path MTU Discovery (IP level, PMTUD_STATE). */
		PMTUD = IP_MTU_DISCOVER
#endif
	};

#ifdef _WIN32
	/* Gets the size (in bytes) of the buffer required to get a specific socket option. */
	_Check_return_ inline size_t win32GetSockOptionBufferSize(_In_ SocketOption option)
	{
		switch (option)
		{
		case SocketOption::Broadcast:
		case SocketOption::ConditionalAccept:
		case SocketOption::Debug:
		case SocketOption::DontLinger:
		case SocketOption::DontRoute:
		case SocketOption::ExclusiveAddressUse:
		case SocketOption::KeepAlive:
		case SocketOption::OutOfBandInline:
		case SocketOption::ReuseAddress:
		case SocketOption::DontFragment:
			return sizeof(bool);
		case SocketOption::Linger:
			return sizeof(LINGER);
		case SocketOption::ReceiveBuffer:
		case SocketOption::SendBuffer:
			return sizeof(int);
		case SocketOption::ReceiveTimeout:
		case SocketOption::SendTimeout:
		case SocketOption::MTU:
		case SocketOption::PMTUD:
			return sizeof(DWORD);
		}

		return 0;
	}

	/* Gets the socket level associated with the specific option. */
	_Check_return_ inline int win32GetSockOptionLevel(_In_ SocketOption option)
	{
		switch (option)
		{
		case SocketOption::Broadcast:
		case SocketOption::ConditionalAccept:
		case SocketOption::DontLinger:
		case SocketOption::DontRoute:
		case SocketOption::ExclusiveAddressUse:
		case SocketOption::KeepAlive:
		case SocketOption::Linger:
		case SocketOption::OutOfBandInline:
		case SocketOption::ReceiveBuffer:
		case SocketOption::ReceiveTimeout:
		case SocketOption::ReuseAddress:
		case SocketOption::SendBuffer:
		case SocketOption::SendTimeout:
			return SOL_SOCKET;
		case SocketOption::NoDelay:
			return IPPROTO_TCP;
		case SocketOption::DontFragment:
		case SocketOption::MTU:
		case SocketOption::PMTUD:
			return IPPROTO_IP;
		}

		return -1;
	}
#endif
}