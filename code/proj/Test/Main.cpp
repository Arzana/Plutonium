#include "TestGame.h"

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

#if defined(DEBUG)
	_CrtPressAnyKeyToContinue();
#endif
	return 0;
}