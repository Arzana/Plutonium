#include <Game.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Vector3.h>

using namespace Plutonium;

struct TestGame
	: public Game
{
	FpsCounter *fps;

	TestGame(void)
		: Game("TestGame")
	{}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
	}

	virtual void LoadContent(void)
	{}

	virtual void UnLoadContent(void)
	{}

	virtual void Finalize(void)
	{}

	virtual void Update(_In_ float dt)
	{}

	virtual void Render(_In_ float dt)
	{
		LOG("\n- Ups (cur): %f.\n- Fps (cur): %f.\n- Fps (avg): %f.", fps->GetUps(), fps->GetCurFps(), fps->GetAvrgFps());
	}

	virtual void RenderLoad(_In_ float dt, _In_ int percentage)
	{}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

	return 0;
}