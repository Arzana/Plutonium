#pragma once
#include "IPAddress.h"
#include "AddressFamily.h"
#include "Protocol.h"
#include "SocketType.h"
#include "SocketOption.h"
#include <WS2tcpip.h>

namespace Pu
{
	/* Defines an object to create, send and receive packets from a network socket. */
	class Socket
	{
	public:
		/* Initializes a new instance of a socket. */
		Socket(_In_ AddressFamily family, _In_ SocketType type, _In_ Protocol protocol);
		Socket(_In_ const Socket&) = delete;
		/* Move constructor. */
		Socket(_In_ Socket &&value);
		/* Releases the socket to the OS. */
		~Socket(void)
		{
			Destroy();
		}

		_Check_return_ Socket& operator =(_In_ const Socket&) = delete;
		/* Move assignment. */
		_Check_return_ Socket& operator =(_In_ Socket &&other);

		/* Gets the address family of the socket. */
		_Check_return_ inline AddressFamily GetFamily(void) const
		{
			return family;
		}

		/* Gets the sockets type. */
		_Check_return_ inline SocketType GetType(void) const
		{
			return type;
		}

		/* Gets the protocol of the socket. */
		_Check_return_ inline Protocol GetProtocol(void) const
		{
			return protocol;
		}

		/* Gets whether the socket was bound to a local port. */
		_Check_return_ inline bool IsBound(void) const
		{
			return bound;
		}

		/* Gets whether the socket is connected to a remote host. */
		_Check_return_ inline bool IsConnected(void) const
		{
			return connected;
		}

		/* Sets a specific socket level option. */
		void SetOption(_In_ SocketOption option, _In_ const void *value);
		/* Gets the value of a specific socket level option (returns whether the opteration was successful). */
		_Check_return_ bool GetOption(_In_ SocketOption option, _In_ void *result) const;
		/* Checks whether there is data available for the socket to read. */
		_Check_return_ bool Poll(void) const;
		/* Binds the socket to a local endpoint. */
		void Bind(_In_ IPAddress address, _In_ uint16 port);
		/* Closes the socket. */
		void Close(void);
		/* Establish a connection to a remote endpoint. */
		void Connect(_In_ IPAddress address, _In_ uint16 port);
		/* Closes the connection to the remote endpoint. */
		void Disconnect(void);
		/* Sends the specified packet to the connected socket. */
		void Send(_In_ const string &packet);
		/* Sends the specified packet to the specified endpoint. */
		void Send(_In_ const string &packet, _In_ IPAddress address, _In_ uint16 port);
		/* Reveives a datagram from the connected endpoint (returns the amount of bytes read). */
		_Check_return_ size_t Receive(_In_ void *data, _In_ size_t bufferSize);
		/* Receives a datagram from the specified endpoint (returns the amount of bytes read). */
		_Check_return_ size_t Receive(_In_ void *data, _In_ size_t bufferSize, _In_ IPAddress address, _In_ uint16 port);

		/* Set the version of the underlying protocol (WSA for Win32). */
		static void SetVersion(int major, int minor, int patch);
	private:
		AddressFamily family;
		SocketType type;
		Protocol protocol;
		bool bound;
		bool connected;

#ifdef _WIN32
		static bool initWSA;
		static uint16 versionWSA;

		uint64 hndl;

		LPADDRINFO GetSockAddr(IPAddress address, uint16 port) const;
		static void InitializeWSA(void);
		static wstring GetLastWSAError(void);
#endif

		void Destroy(void);
	};
}