#include <Game.h>
#include <Graphics\Text\DebugTextRenderer.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Graphics\Rendering\DynamicRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Basics.h>
#include <Core\String.h>
#include "KnightInit.h"

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *fontRenderer;
	StaticRenderer *srenderer;
	DynamicRenderer *drenderer;
	Camera *cam;

	/* Scene */
	StaticModel *heart;
	DynamicModel *knight;
	Vector3 light = Vector3::Zero;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;

	TestGame(void)
		: Game("TestGame")
	{
		GetGraphics()->GetWindow()->SetMode(WindowMode::BorderlessFullscreen);
		GetCursor()->Disable();
	}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		AddComponent(mem = new MemoryCounter(this));

		fontRenderer = new DebugFontRenderer(GetGraphics(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Debug_Text.vsh", "./assets/shaders/Debug_Text.fsh");
		srenderer = new StaticRenderer("./assets/shaders/Static3D.vsh", "./assets/shaders/Static3D.fsh");
		drenderer = new DynamicRenderer("./assets/shaders/Dynamic3D.vsh", "./assets/shaders/Static3D.fsh");
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());

		heart = StaticModel::FromFile("./assets/models/Heart/Heart.obj");
		heart->SetScale(10.0f);

		knight = DynamicModel::FromFile("./assets/models/Knight/knight.md2", "knight.bmp");
		knight->SetOrientation(-PI2, -PI2, 0.0f);
		knight->Initialize(InitKnight);
		knight->PlayAnimation("stand");
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(heart);
		delete_s(knight);
	}

	virtual void Finalize(void)
	{
		delete_s(fontRenderer);
		delete_s(srenderer);
		delete_s(drenderer);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		static float theta = 0.0f;
		theta = modrads(theta += DEG2RAD * dt * 100);
		light = Vector3::FromYaw(theta);
		String lightStr = "Light ";
		fontRenderer->AddDebugString((lightStr += ipart(theta * RAD2DEG)) += "°");

		/* Update scene. */
		knight->Update(dt);

		/* Update camera. */
		cam->Update(dt, GetKeyboard(), GetCursor());

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

		/* Render static models. */
		srenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		//srenderer->Render(heart);
		srenderer->End();

		/* Render dynamic models. */
		drenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		drenderer->Render(knight);
		drenderer->End();

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