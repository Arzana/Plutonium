#include <Game.h>
#include <Components\FpsCounter.h>
#include <Graphics\Text\DebugTextRenderer.h>
#include <Core\String.h>

using namespace Plutonium;

struct TestGame
	: public Game
{
	FpsCounter *fps;
	DebugFontRenderer *fontRenderer;

	TestGame(void)
		: Game("TestGame")
	{}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		fontRenderer = new DebugFontRenderer(GetWindow(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Debug_Text.vsh", "./assets/shaders/Debug_Text.fsh");
	}

	virtual void LoadContent(void)
	{}

	virtual void UnLoadContent(void)
	{}

	virtual void Finalize(void)
	{
		delete_s(fontRenderer);
	}

	virtual void Update(float dt)
	{}

	virtual void Render(float dt)
	{
		/* Render current UPS. */
		String upsStr = "Ups (cur): ";
		upsStr += int(fps->GetUps());
		fontRenderer->AddDebugString(upsStr);

		/* Render current FPS. */
		String fpscStr = "Fps (cur): ";
		fpscStr += int(fps->GetCurFps());
		fontRenderer->AddDebugString(fpscStr);

		/* Render average FPS. */
		String fpsaStr = "Fps (avg): ";
		fpsaStr += int(fps->GetAvrgFps());
		fontRenderer->AddDebugString(fpsaStr);

		fontRenderer->Render();
	}

	virtual void RenderLoad(float dt, int percentage)
	{}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

	return 0;
}