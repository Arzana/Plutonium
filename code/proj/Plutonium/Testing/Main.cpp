#include "TestGame.h"

int main(int, char**)
{
	TestGame *game = new TestGame();
	game->Run();
	delete game;
	return 0;
}