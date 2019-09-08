#include "Networking/Core/Socket.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

#ifdef _WIN32
bool Pu::Socket::initWSA = false;
Pu::uint16 Pu::Socket::versionWSA = WINSOCK_VERSION;
#endif

#ifdef _WIN32
Pu::Socket::Socket(AddressFamily family, SocketType type, Protocol protocol)
	: family(family), type(type), protocol(protocol), bound(false), connected(false)
{
	/* Make sure the API is initialized. */
	InitializeWSA();

	/* Create the desired socket. */
	hndl = socket(static_cast<int>(family), static_cast<int>(type), static_cast<int>(protocol));
	if (hndl == INVALID_SOCKET) Log::Error("Failed to initialize socket (%ls)!", GetLastWSAError().c_str());
}
#endif

Pu::Socket::Socket(Socket && value)
	: family(value.family), type(value.type), protocol(value.protocol), bound(value.bound), connected(value.connected)
#ifdef _WIN32
	, hndl(value.hndl)
#endif
{
#ifdef _WIN32
	value.hndl = INVALID_SOCKET;
#endif

	value.bound = false;
	value.connected = false;
}

Pu::Socket & Pu::Socket::operator=(Socket && other)
{
	if (this != &other)
	{
		Destroy();
		family = other.family;
		type = other.type;
		protocol = other.protocol;
		bound = other.bound;
		connected = other.connected;

#ifdef _WIN32
		hndl = other.hndl;
		other.hndl = INVALID_SOCKET;
#endif

		other.bound = false;
		other.connected = false;
	}

	return *this;
}

void Pu::Socket::SetOption(SocketOption option, const void * value)
{
#ifdef _WIN32
	/* Quick error check. */
	if (hndl == INVALID_SOCKET)
	{
		Log::Error("Cannot set socket option on invalid socket!");
		return;
	}

	/* Single out the TCP socket option. */
	if (option == SocketOption::NoDelay)
	{
		if (protocol == Protocol::TCP) setsockopt(hndl, IPPROTO_TCP, static_cast<int>(option), reinterpret_cast<const char*>(value), sizeof(bool));
		else Log::Error("Cannot set NoDelay option on non-TCP socket!");
	}
	else
	{
		/* Get the byte length for the specified option and apply it. */
		const int optlen = static_cast<int>(win32GetSockOptionBufferSize(option));
		setsockopt(hndl, SOL_SOCKET, static_cast<int>(option), reinterpret_cast<const char*>(value), optlen);
	}
#else
	Log::Warning("Cannot set socket option on this platform!");
#endif
}

bool Pu::Socket::GetOption(SocketOption option, void * result) const
{
#ifdef _WIN32
	/* Quick error check. */
	if (hndl == INVALID_SOCKET)
	{
		Log::Error("Cannot get socket option on invalid socket!");
		return false;
	}

	int code;
	int optLen = static_cast<int>(win32GetSockOptionBufferSize(option));

	/* Single out the TCP socket option. */
	if (option == SocketOption::NoDelay)
	{
		if (protocol == Protocol::TCP) code = getsockopt(hndl, IPPROTO_TCP, static_cast<int>(option), reinterpret_cast<char*>(result), &optLen);
		else
		{
			Log::Error("Cannot check NoDelay option on non-TCP socket!");
			return false;
		}
	}
	else code = getsockopt(hndl, SOL_SOCKET, static_cast<int>(option), reinterpret_cast<char*>(result), &optLen);

	/* Return the result and do a final error check. */
	if (code != SOCKET_ERROR) return true;
	else Log::Error("Unable to get socket option (%ls)!", GetLastWSAError().c_str());
#else
	Log::Warning("Cannot get socket option on this platform!");
#endif

	return false;
}

bool Pu::Socket::Poll(void) const
{
#ifdef _WIN32
	/* Just return false for invalid sockets. */
	if (hndl != INVALID_SOCKET)
	{
		/* Poll the socket for all incomming (broadcast and direct) messages. */
		pollfd info = { hndl, POLLRDNORM };
		const int code = WSAPoll(&info, 1, 0);
		if (code != SOCKET_ERROR) return code > 0;

		Log::Error("Socket poll failed (%ls)!", GetLastWSAError().c_str());
	}
#else
	Log::Warning("Socket polling is not supported on this platform!");
#endif

	return false;
}

void Pu::Socket::Bind(IPAddress address, uint16 port)
{
#ifdef _WIN32
	if (hndl == INVALID_SOCKET || bound)
	{
		Log::Warning("Unable to bind invalid socket!");
		return;
	}

	/* Get the address info from the supplied data. */
	LPADDRINFO info = GetSockAddr(address, port);
	if (info)
	{
		/* Actually bind the socket. */
		const int code = bind(hndl, info->ai_addr, static_cast<int>(info->ai_addrlen));
		if (code == SOCKET_ERROR) Log::Error("Unable to bind socket (%ls)!", GetLastWSAError().c_str());
		else bound = true;

		freeaddrinfo(info);
	}
	else Log::Error("Unable to bind socket (could not get address info)!");
#else
	Log::Warning("Cannot bind socket on this platform!");
#endif
}

void Pu::Socket::Close(void)
{
#ifdef _WIN32
	if (hndl != INVALID_SOCKET)
	{
		closesocket(hndl);
		hndl = INVALID_SOCKET;
	}
#else
	Log::Warning("COuld not close socket on this platform!");
#endif
}

void Pu::Socket::Connect(IPAddress address, uint16 port)
{
#ifdef _WIN32
	if (hndl == INVALID_SOCKET || connected)
	{
		Log::Warning("Unable to connect with invalid socket!");
		return;
	}

	LPADDRINFO info = GetSockAddr(address, port);
	if (info)
	{
		/* Connect to the remote host. */
		const int code = connect(hndl, info->ai_addr, static_cast<int>(info->ai_addrlen));
		if (code == SOCKET_ERROR) Log::Error("Unable to connect to remote host (%ls)!", GetLastWSAError().c_str());
		else connected = true;

		freeaddrinfo(info);
	}
	else Log::Error("Could not connect to remote host (could not get address info)!");
#else
	Log::Warning("Cannot connect to remote host on this platform!");
#endif
}

void Pu::Socket::Disconnect(void)
{
#ifdef _WIN32
	if (connected)
	{
		connected = false;
		shutdown(hndl, SD_BOTH);
	}
#else
	Log::Warning("Unable to disconnect socket on this platform!");
#endif
}

void Pu::Socket::Send(const string & packet)
{
	/* Check if we can even call this function. */
	if (!connected)
	{
		Log::Error("Cannot send packet on a non-connected socket!");
		return;
	}

	/* Send the packet to the connection. */
#ifdef _WIN32
	if (hndl != INVALID_SOCKET)
	{
		if (send(hndl, packet.c_str(), static_cast<int>(packet.size()), 0) == SOCKET_ERROR)
		{
			Log::Error("Sending packet to connected endpoint failed (%ls)!", GetLastWSAError().c_str());
		}
	}
	else Log::Error("Could not send packed on invalid socket!");
#else
	Log::Warning("Sending to a connection is not supported on this platform!");
#endif
}

void Pu::Socket::Send(const string & packet, IPAddress address, uint16 port)
{
#ifdef _WIN32
	if (hndl != INVALID_SOCKET)
	{
		/* sockaddr_in should be used because it gives us the full endpoint for the receiver. */
		sockaddr_in endp;
		endp.sin_family = static_cast<short>(family);
		endp.sin_port = htons(port);
		endp.sin_addr.s_addr = static_cast<ULONG>(address.GetAddress());

		if (sendto(hndl, packet.c_str(), static_cast<int>(packet.size()), 0, reinterpret_cast<sockaddr*>(&endp), sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			Log::Error("Sending packet to specific endpoint failed (%ls)!", GetLastWSAError().c_str());
		}
	}
	else Log::Error("Could not send packet on invalid socket!");
#else
	Log::Warning("Sending packets is not supported on this platform!");
#endif
}

size_t Pu::Socket::Receive(void * data, size_t bufferSize)
{
	/* Early out if there is no packet to receive. */
	if (Poll())
	{
#ifdef _WIN32
		if (hndl != INVALID_SOCKET)
		{
			/* Receive the pending package and check for errors. */
			const int result = recv(hndl, reinterpret_cast<char*>(data), static_cast<int>(bufferSize), 0);
			if (result == SOCKET_ERROR)
			{
				Log::Error("Receiving packet failed (%ls)!", GetLastWSAError().c_str());
				return 0;
			}

			/* Returns the amount of bytes in the message. */
			return static_cast<size_t>(result);
		}
		else Log::Error("Could not receive packet on invalid socket!");
#else
		Log::Warning("Unable to receive packet on this platform!");
#endif
	}

	return 0;
}

size_t Pu::Socket::Receive(void * data, size_t bufferSize, IPAddress address, uint16 port)
{
	/* Eearly out if there is no packet to receive. */
	if (Poll())
	{
#ifdef _WIN32
		if (hndl != INVALID_SOCKET)
		{
			/* sockaddr_in should be used because it gives us the full endpoint for the sender. */
			sockaddr_in endp;
			endp.sin_family = static_cast<short>(family);
			endp.sin_port = htons(port);
			endp.sin_addr.s_addr = static_cast<ULONG>(address.GetAddress());

			/* Get the packet from the specified sender. */
			int len = sizeof(sockaddr_in);
			const int result = recvfrom(hndl, reinterpret_cast<char*>(data), static_cast<int>(bufferSize), 0, reinterpret_cast<sockaddr*>(&endp), &len);

			/* Check for errors. */
			if (result == SOCKET_ERROR)
			{
				Log::Error("Receiving packet failed (%ls)!", GetLastWSAError().c_str());
				return 0;
			}

			return static_cast<size_t>(result);
		}
		else Log::Error("Could not receive packet from specific endpoint on invalid socket!");
#else
		Log::Warning("Unable to receive packet from specific endpoint on this platform!");
#endif
	}

	return 0;
}

void Pu::Socket::SetVersion(int major, int minor, int patch)
{
#ifdef _WIN32
	/* patch version is not used for Windows. */
	(void)patch;
	versionWSA = MAKEWORD(major, minor);
#endif
}

#ifdef _WIN32
LPADDRINFO Pu::Socket::GetSockAddr(IPAddress address, uint16 port) const
{
	addrinfo hints, *result;
	ZeroMemory(&hints, sizeof(addrinfo));
	hints.ai_family = static_cast<int>(family);
	hints.ai_socktype = static_cast<int>(type);
	hints.ai_protocol = static_cast<int>(protocol);

	/* Get the address and port to bind to. */
	int code = getaddrinfo(address.GetAddressStr().c_str(), string::from(port).c_str(), &hints, &result);
	if (code)
	{
		Log::Error("Unable to get address information (%ls)!", _CrtFormatError(code).c_str());
		return nullptr;
	}

	return result;
}

void Pu::Socket::InitializeWSA(void)
{
	/* Make sure we don't do it multiple times. */
	if (initWSA) return;

	/* Initialize the Windows API. */
	WSADATA data;
	const int code = WSAStartup(versionWSA, &data);

	/* Check for errors. */
	if (code) Log::Error("Failed to initialize Windows socket API (%ls)!", _CrtFormatError(code).c_str());
	else initWSA = true;
}

Pu::wstring Pu::Socket::GetLastWSAError(void)
{
	return _CrtFormatError(WSAGetLastError());
}
#endif

void Pu::Socket::Destroy(void)
{
	Disconnect();
	Close();
}