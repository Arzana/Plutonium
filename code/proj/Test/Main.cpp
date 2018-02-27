#include <Game.h>
#include <Graphics\Text\DebugTextRenderer.h>
#include <Graphics\Rendering\Renderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Basics.h>
#include <Core\String.h>

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *fontRenderer;
	Renderer *renderer;
	Camera *cam;

	/* Scene */
	Model *heart;
	Vector3 light = Vector3::Zero;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;

	TestGame(void)
		: Game("TestGame")
	{}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		AddComponent(mem = new MemoryCounter(this));

		fontRenderer = new DebugFontRenderer(GetWindow(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Debug_Text.vsh", "./assets/shaders/Debug_Text.fsh");
		renderer = new Renderer("./assets/shaders/Basic3D.vsh", "./assets/shaders/Basic3D.fsh");
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetWindow());

		heart = Model::FromFile("./assets/models/Heart/Heart.obj");
		heart->SetScale(10.0f);
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(heart);
	}

	virtual void Finalize(void)
	{
		delete_s(fontRenderer);
		delete_s(renderer);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		static float theta = 0.0f;
		theta = modrads(theta += DEG2RAD * dt * 100);
		light = Vector3::FromYaw(theta);
		String lightStr = "Light ";
		fontRenderer->AddDebugString((lightStr += ipart(theta * RAD2DEG)) += "°");

		/* Update camera. */
		cam->Update(dt, heart->GetWorld());

		/* Update input. */
		if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
	}

	virtual void Render(float dt)
	{
		/* Render average FPS. */
		String fpsaStr = "Fps (avg): ";
		fontRenderer->AddDebugString(fpsaStr += ipart(fps->GetAvrgHz()));

		/* Render average VRAM. */
		String vramStr = "VRAM: ";
		(vramStr += b2mb(mem->GetAvrgVRamUsage())) += " / ";
		(vramStr += b2mb(mem->GetOSVRamBudget())) += " MB";
		fontRenderer->AddDebugString(vramStr);

		/* Render models. */
		renderer->Begin(cam->GetView(), cam->GetProjection(), light);
		renderer->Render(heart);
		renderer->End();

		/* Render text. */
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