#include "TestGame.h"

int main(int, char**)
{
	TestGame *game = new TestGame();
	game->Run();
	delete game;

	Pu::Log::PressAnyKeyToContinue();
	return 0;
}