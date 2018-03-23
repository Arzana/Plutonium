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
	DynamicRenderer *drenderer;
	Camera *cam;

	/* Scene */
	float theta;
	DynamicModel *knight;
	Vector3 light = Vector3::Zero;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;

	TestGame(void)
		: Game("TestGame"), theta(0.0f)
	{
		GetGraphics()->GetWindow()->SetMode(WindowMode::BorderlessFullscreen);
		GetCursor()->Disable();
	}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		AddComponent(mem = new MemoryCounter(this));

		fontRenderer = new DebugFontRenderer(GetGraphics(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vsh", "./assets/shaders/Text2D.fsh");
		drenderer = new DynamicRenderer("./assets/shaders/Dynamic3D.vsh", "./assets/shaders/Static3D.fsh");
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());

		knight = DynamicModel::FromFile("./assets/models/Knight/knight.md2", "knight.bmp");
		knight->SetOrientation(-PI2, -PI2, 0.0f);
		knight->Initialize(InitKnight);
		knight->PlayAnimation("stand");
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(knight);
	}

	virtual void Finalize(void)
	{
		delete_s(fontRenderer);
		delete_s(drenderer);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		theta = modrads(theta += DEG2RAD * dt * 100);
		light = Vector3::FromYaw(theta);

		/* Update scene. */
		knight->Update(dt);

		/* Update camera. */
		cam->Update(dt, GetKeyboard(), GetCursor());

		/* Update input. */
		if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
	}

	virtual void PreRender(void)
	{
		/* If the knight model is used face culling needs to be turned off. */
		GetGraphics()->SetFaceCull(FaceCullState::None);
	}

	virtual void Render(float dt)
	{
		/* Render light direction. */
		std::string lightStr = "Light ";
		lightStr += std::to_string(ipart(theta * RAD2DEG)) += '°';
		fontRenderer->AddDebugString(lightStr);

		/* Render average FPS. */
		std::string fpsaStr = "Fps (avg): ";
		fpsaStr += std::to_string(ipart(fps->GetAvrgHz()));
		fontRenderer->AddDebugString(fpsaStr);

		/* Render average VRAM. */
		std::string vramStr = "VRAM: ";
		(vramStr += std::to_string(b2mb(mem->GetAvrgVRamUsage()))) += " / ";
		(vramStr += std::to_string(b2mb(mem->GetOSVRamBudget()))) += " MB";
		fontRenderer->AddDebugString(vramStr);

		/* Render dynamic models. */
		drenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		drenderer->Render(knight);
		drenderer->End();

		/* Render text. */
		fontRenderer->Render();
	}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

	_CrtPressAnyKeyToContinue();
	return 0;
}