#include "Networking/Core/IPAddress.h"
#include "Core/Platform/Windows/Windows.h"
#include <WS2tcpip.h>

Pu::IPAddress::IPAddress(uint64 address)
	: address(address)
{}

Pu::IPAddress::IPAddress(octet nw1, octet nw2, octet nw3, octet host)
	: address(host)
{
	address |= (nw3 << 0x8);
	address |= (nw2 << 0x10);
	address |= (nw1 << 0x18);
}

Pu::IPAddress::IPAddress(const string & address)
{
	inet_pton(AF_INET, address.c_str(), &this->address);
}

Pu::IPAddress Pu::IPAddress::GetAny(void)
{
	return IPAddress(INADDR_ANY);
}

Pu::IPAddress Pu::IPAddress::GetBroadcast(void)
{
	return IPAddress(INADDR_BROADCAST);
}

Pu::IPAddress Pu::IPAddress::GetLoopback(void)
{
	return IPAddress(INADDR_LOOPBACK);
}

Pu::string Pu::IPAddress::GetAddressStr(void) const
{
	const octet host = static_cast<octet>(address & 0x000000FF);
	const octet nw3 = static_cast<octet>((address & 0x0000FF00) >> 0x8);
	const octet nw2 = static_cast<octet>((address & 0x00FF0000) >> 0x10);
	const octet nw1 = static_cast<octet>((address & 0xFF000000) >> 0x18);

	/* Preallocating the buffer saves performance. */
	string buffer(16ull);
	snprintf(buffer.data(), buffer.size(), "%d.%d.%d.%d", nw1, nw2, nw3, host);
	return buffer;
}