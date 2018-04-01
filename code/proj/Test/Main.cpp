#include <string>
#include <Game.h>
#include <Core\String.h>
#include <Graphics\Diagnostics\DebugTextRenderer.h>
#include <Graphics\Diagnostics\DebugSpriteRenderer.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Graphics\Portals\PortalRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Basics.h>
#include <Graphics\Diagnostics\FrameInfo.h>

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *dfRenderer;
	DebugSpriteRenderer *dsRenderer;
	StaticRenderer *srenderer;
	PortalRenderer *prenderer;
	Camera *cam;

	/* Scene */
	float theta;
	std::vector<EuclidRoom*> Map;
	Vector3 light = Vector3::Zero;
	size_t curRoom = 0;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;
	Texture *depthSprite, *stencilSprite;

	TestGame(void)
		: Game("TestGame"), theta(0.0f), depthSprite(nullptr), stencilSprite(nullptr)
	{
		Window *wnd = GetGraphics()->GetWindow();
		wnd->Move(Vector2::Zero);
		wnd->Resize(wnd->GetGraphicsDevice().GetClientSize());

		GetCursor()->Disable();
	}

	virtual void Initialize(void)
	{
		AddComponent(fps = new FpsCounter(this, 100, 1));
		AddComponent(mem = new MemoryCounter(this, 100, 1));

		dfRenderer = new DebugFontRenderer(GetGraphics(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vsh", "./assets/shaders/Text2D.fsh");
		dsRenderer = new DebugSpriteRenderer(GetGraphics(), "./assets/shaders/Static2D.vsh", "./assets/shaders/Static2D.fsh", Vector2(0.0f, 100.0f));
		srenderer = new StaticRenderer("./assets/shaders/Static3D.vsh", "./assets/shaders/Static3D.fsh");
		prenderer = new PortalRenderer(GetGraphics(), "./assets/shaders/StencilOnly3D.vsh");
		prenderer->OnRoomRender.Add(this, &TestGame::RenderScene);
		GetKeyboard()->KeyPress.Add(this, &TestGame::KeyInput);
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());

		Map = EuclidRoom::FromFile("assets/models/Maps/dm_arena.pobj");
		for (size_t i = 0; i < Map.size(); i++) Map.at(i)->SetScale(10.0f);
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);

		while (Map.size() > 0)
		{
			EuclidRoom *cur = Map.back();
			delete_s(cur);
			Map.pop_back();
		}
	}

	virtual void Finalize(void)
	{
		prenderer->OnRoomRender.Remove(this, &TestGame::RenderScene);
		GetKeyboard()->KeyPress.Remove(this, &TestGame::KeyInput);

		delete_s(dfRenderer);
		delete_s(dsRenderer);
		delete_s(srenderer);
		delete_s(prenderer);
		if (depthSprite) delete_s(depthSprite);
		if (depthSprite) delete_s(stencilSprite);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		theta = modrads(theta += DEG2RAD * dt * 100);
		light = Vector3::FromYaw(theta);

		/* Update camera. */
		cam->Update(dt, GetKeyboard(), GetCursor());

		/* Update input. */
		if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
	}

	void KeyInput(WindowHandler, KeyEventArgs args)
	{
#if defined(DEBUG)
		/* If PrintScreen is pressed (once), update frame diagnostics. */
		if (args.Key == Keys::PrintScreen && args.Action == KeyState::Down)
		{
			/* Create new texture from depth and delete old texture if needed. */
			if (depthSprite) delete_s(depthSprite);
			depthSprite = _CrtSaveDepthToTexture(GetGraphics());

			/* Create new texture from stencil and delete old texture is needed. */
			if (stencilSprite) delete_s(stencilSprite);
			stencilSprite = _CrtSaveStencilToTexture(GetGraphics());

			/* Operation will take a long time, so make sure it doesn't affect next frames delta. */
			SuppressNextUpdate();
		}
#endif
	}

	void RenderScene(const PortalRenderer*, SceneRenderArgs args)
	{
		/* Left wall of blue room is broken so we need to disable face culling. */
		GetGraphics()->SetFaceCull(FaceCullState::None);

		size_t idx = -1;
		for (size_t i = 0; i < Map.size(); i++)
		{
			if (Map.at(i)->GetID() == args.SceneID)
			{
				idx = i;
				break;
			}
		}

		srenderer->Begin(args.View, args.Projection, light);
		srenderer->Render(Map.at(idx));
		srenderer->End();
	}

	virtual void Render(float dt)
	{
		GetGraphics()->SetFaceCull(FaceCullState::None);

		srenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		srenderer->Render(Map.at(curRoom));
		srenderer->End();

		/* Render scene. */
		Tree<PortalRenderArgs> portals;
		Map.at(curRoom)->AddPortals(&portals);
		prenderer->Render(cam->GetView(), cam->GetProjection(), &portals);

		/* Render frame buffer diagnostics. */
		dsRenderer->Begin();
		if (depthSprite) dsRenderer->RenderDebug(depthSprite, Color::White, Vector2(0.1f));
		if (stencilSprite) dsRenderer->RenderDebug(stencilSprite, Color::White, Vector2(0.1f));
		dsRenderer->End();

		/* Render light direction. */
		std::string lightStr = "Light: ";
		lightStr += std::to_string(ipart(theta * RAD2DEG)) += '°';
		dfRenderer->AddDebugString(lightStr);

		/* Render average FPS. */
		std::string fpsaStr = "Fps (avg): ";
		fpsaStr += std::to_string(ipart(fps->GetAvrgHz()));
		dfRenderer->AddDebugString(fpsaStr);

		/* Render average VRAM. */
		std::string ramStr = "RAM: ";
		((ramStr += b2short_string(mem->GetAvrgRamUsage())) += " / ") += b2short_string(mem->GetOSRamBudget());
		dfRenderer->AddDebugString(ramStr);
		dfRenderer->Render();
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