#include <string>
#include <Game.h>
#include <Graphics\Text\DebugTextRenderer.h>
#include <Graphics\Rendering\DynamicRenderer.h>
#include <Graphics\Portals\PortalRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Basics.h>
#include "KnightInit.h"

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *fRenderer;
	DynamicRenderer *drenderer;
	PortalRenderer *prenderer;
	Camera *cam;

	/* Scene */
	float theta;
	DynamicModel *knight;
	Portal *portal;
	Vector3 light = Vector3::Zero;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;

	TestGame(void)
		: Game("TestGame"), theta(0.0f)
	{
		GetCursor()->Disable();
	}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this));
		AddComponent(mem = new MemoryCounter(this));

		fRenderer = new DebugFontRenderer(GetGraphics(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vsh", "./assets/shaders/Text2D.fsh");
		drenderer = new DynamicRenderer("./assets/shaders/Dynamic3D.vsh", "./assets/shaders/Static3D.fsh");
		prenderer = new PortalRenderer(GetGraphics(), "./assets/shaders/StencilOnly3D.vsh");
		prenderer->OnRoomRender.Add(this, &TestGame::RenderScene);
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());

		knight = DynamicModel::FromFile("./assets/models/Knight/knight.md2", "knight.bmp");
		knight->Teleport(Vector3::Forward * 10.0f);
		knight->SetOrientation(-PI2, -PI2, 0.0f);
		knight->Initialize(InitKnight);
		knight->PlayAnimation("stand");

		portal = new Portal(Vector3::Zero);
		portal->Destination = knight;
		portal->SetScale(10.0f);
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(knight);
	}

	virtual void Finalize(void)
	{
		prenderer->OnRoomRender.Remove(this, &TestGame::RenderScene);

		delete_s(fRenderer);
		delete_s(drenderer);
		delete_s(prenderer);
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

	void RenderScene(const PortalRenderer *sender, SceneRenderArgs args)
	{
		drenderer->Begin(args.View, args.Projection, light);
		drenderer->Render(knight);
		drenderer->End();

		std::string distStr = "Portal distance: ";
		distStr += std::to_string(ipart(dist(cam->GetPosition(), args.View.GetTranslation())));
		fRenderer->AddDebugString(distStr);
	}

	virtual void Render(float dt)
	{
		/* Render light direction. */
		std::string lightStr = "Light: ";
		lightStr += std::to_string(ipart(theta * RAD2DEG)) += '°';
		fRenderer->AddDebugString(lightStr);

		/* Render distance to knight. */
		std::string distStr = "Distance ";
		distStr += std::to_string(ipart(dist(cam->GetPosition(), knight->GetPosition())));
		fRenderer->AddDebugString(distStr);

		/* Render average FPS. */
		std::string fpsaStr = "Fps (avg): ";
		fpsaStr += std::to_string(ipart(fps->GetAvrgHz()));
		fRenderer->AddDebugString(fpsaStr);

		/* Render average VRAM. */
		std::string vramStr = "VRAM: ";
		(vramStr += std::to_string(b2mb(mem->GetAvrgVRamUsage()))) += " / ";
		(vramStr += std::to_string(b2mb(mem->GetOSVRamBudget()))) += " MB";
		fRenderer->AddDebugString(vramStr);

		/* If the knight model is used face culling needs to be turned off. */
		GetGraphics()->SetFaceCull(FaceCullState::None);

		/* Render test current room. */
		drenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		//drenderer->Render(knight);
		drenderer->End();

		/* Render scene. */
		Tree<PortalRenderArgs> portals;
		portals.Add({ 1, portal });
		prenderer->Render(cam->GetView(), cam->GetProjection(), &portals);

		/* Render text. */
		fRenderer->Render();
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