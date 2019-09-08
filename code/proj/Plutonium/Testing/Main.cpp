#include <Networking/UPnP/SSDP.h>
#include "TestGame.h"

int main(int, char**)
{
	{
		Pu::Socket socket{ Pu::AddressFamily::IPv4, Pu::SocketType::Dgram, Pu::Protocol::UDP };
		Pu::SSDP ssdp{ socket };

		ssdp.Discover("upnp:rootdevice");
		bool waiting = true;
		do
		{
			waiting = !ssdp.PollResponse();
		} while (waiting);

		const Pu::SSDPResponse &response = ssdp.GetLastResponse();
		Pu::Log::Verbose("Found rootdevice on %s!", response.ServerName.c_str());
	}

	TestGame *game = new TestGame();
	game->Run();
	delete game;
	return 0;
}