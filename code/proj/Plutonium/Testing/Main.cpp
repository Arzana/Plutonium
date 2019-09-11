#include <Networking/UPnP/SSDP.h>
#include <Networking/UPnP/HTTPU.h>
#include "TestGame.h"

int main(int, char**)
{
	{
		Pu::Socket socket{ Pu::AddressFamily::IPv4, Pu::SocketType::Dgram, Pu::Protocol::UDP };
		Pu::Socket socket2{ Pu::AddressFamily::IPv4, Pu::SocketType::Stream, Pu::Protocol::TCP };
		Pu::SSDP ssdp{ socket };

		ssdp.Discover("upnp:rootdevice");
		bool waiting = true;
		do
		{
			waiting = !ssdp.PollResponse();
		} while (waiting);

		const Pu::SSDPResponse &response = ssdp.GetLastResponse();
		Pu::Log::Verbose("Found rootdevice on %s!", response.ServerName.c_str());

		Pu::HttpuRequest urlReq{ response.Location };
		urlReq.Send(socket2);

		do
		{
			waiting = !socket2.Poll();
		} while (waiting);

		char buffer[0x1000];
		Pu::HttpuResponse urlResp = Pu::HttpuResponse::Receive(socket2, buffer, 0x1000);

		Pu::Log::Verbose(urlResp.GetBody().c_str());
	}

	TestGame *game = new TestGame();
	game->Run();
	delete game;
	return 0;
}