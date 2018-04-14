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
	Vector3 light;
	std::vector<EuclidRoom*> map;
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
		AddComponent(dfRenderer = new DebugFontRenderer(this, "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vsh", "./assets/shaders/Text2D.fsh"));
		AddComponent(dsRenderer = new DebugSpriteRenderer(this, "./assets/shaders/Static2D.vsh", "./assets/shaders/Static2D.fsh"));

		prenderer = new PortalRenderer(GetGraphics(), "./assets/shaders/PortalFrame3D.vsh");
		prenderer->OnRoomRender.Add(this, &TestGame::RoomThroughPortalRender);

		srenderer = new StaticRenderer("./assets/shaders/Static3D.vsh", "./assets/shaders/Static3D.fsh");
		GetKeyboard()->KeyPress.Add(this, &TestGame::KeyInput);

	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());
		cam->Move(Vector3(20, 5, 10));

		map = EuclidRoom::FromFile("assets/models/Maps/Deathmatch/Warehouse/dm_warehouse.pobj");
		for (size_t i = 0; i < map.size(); i++) map.at(i)->SetScale(2.0f);	// If map is warehouse
		//map = StaticModel::FromFile("assets/models/Sponza/sponza.obj");
		//map->SetScale(0.05f); // If map is sponza
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		for (size_t i = 0; i < map.size(); i++) delete_s(map.at(i));
	}

	virtual void Finalize(void)
	{
		GetKeyboard()->KeyPress.Remove(this, &TestGame::KeyInput);
		prenderer->OnRoomRender.Remove(this, &TestGame::RoomThroughPortalRender);

		delete_s(srenderer);
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

	void RoomThroughPortalRender(const PortalRenderer*, SceneRenderArgs args)
	{
		GetGraphics()->SetFaceCull(FaceCullState::None);

		srenderer->Begin(args.View, args.Projection, light);

		for (size_t i = 0; i < map.size(); i++)
		{
			EuclidRoom *cur = map.at(i);
			if (args.SceneID == cur->GetID())
			{
				srenderer->Render(cur);
				break;
			}
		}

		srenderer->End();
	}

	virtual void Render(float dt)
	{
		GetGraphics()->SetFaceCull(FaceCullState::None);

		/* Render portals. */
		prenderer->Render(cam->GetView(), cam->GetProjection(), map.at(curRoom)->GetPortals());

		/* Render current room last. */
		srenderer->Begin(cam->GetView(), cam->GetProjection(), light);
		srenderer->Render(map.at(curRoom));
		srenderer->End();

		/* Add debug light direction. */
		std::string lightStr = "Light: ";
		lightStr += std::to_string(ipart(theta * RAD2DEG)) += '°';
		dfRenderer->AddDebugString(lightStr);

		/* Add debug average FPS. */
		std::string fpsaStr = "Fps (avg): ";
		fpsaStr += std::to_string(ipart(fps->GetAvrgHz()));
		dfRenderer->AddDebugString(fpsaStr);

		/* Add debug average VRAM. */
		std::string ramStr = "RAM: ";
		((ramStr += b2short_string(mem->GetAvrgRamUsage())) += " / ") += b2short_string(mem->GetOSRamBudget());
		dfRenderer->AddDebugString(ramStr);

		/* Add debug frame buffer diagnostics. */
		if (depthSprite) dsRenderer->AddDebugTexture(depthSprite, Color::White, Vector2(0.1f));
		if (stencilSprite) dsRenderer->AddDebugTexture(stencilSprite, Color::White, Vector2(0.1f));
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