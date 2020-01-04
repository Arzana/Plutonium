#pragma once
#include "WinSock.h"

namespace Pu
{
	/* Defines the type of a socket. */
	enum class SocketType
	{
#ifdef _WIN32
		/* Type of the socket is unknown. */
		Unknown = 0,
		/* Raw transport protocol socket (IP). */
		Raw = SOCK_RAW,
		/* Datagram protocol socket (UDP). */
		Dgram =  SOCK_DGRAM,
		/* Reliably Delivered Messages protocols socket.  */
		Rdm = SOCK_RDM,
		/* Reliable two-way transfer of ordered byte streams. */
		Seqpacket = SOCK_SEQPACKET,
		/* Stream protocol socket (TCP). */
		Stream = SOCK_STREAM
#endif
	};
}