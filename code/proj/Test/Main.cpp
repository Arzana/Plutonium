#include <Game.h>

struct TestGame
	: Plutonium::Game
{
	TestGame(void)
		: Game("TestGame")
	{}

	virtual void Initialize(void) override
	{
	}

	virtual void LoadContent(void) override
	{
	}

	virtual void UnLoadContent(void) override
	{
	}

	virtual void Finalize(void) override
	{
	}

	virtual void Update(float dt) override
	{
	}

	virtual void Render(float dt) override
	{
	}

	virtual void RenderLoad(float dt, int percentage) override
	{
	}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);
	return 0;
}