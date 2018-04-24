#include <Game.h>
#include <Core\String.h>
#include <Graphics\Diagnostics\DebugTextRenderer.h>
#include <Graphics\Diagnostics\DebugSpriteRenderer.h>
#include <Graphics\Diagnostics\FrameInfo.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Basics.h>

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *dfRenderer;
	DebugSpriteRenderer *dsRenderer;
	StaticRenderer *srenderer;
	Camera *cam;

	/* Scene */
	float theta;
	Vector3 light;
	StaticModel *map;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;
	Texture *depthSprite;

	TestGame(void)
		: Game("TestGame"), theta(0.0f), depthSprite(nullptr)
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
		AddComponent(dfRenderer = new DebugFontRenderer(this, "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vert", "./assets/shaders/Text2D.frag"));
		AddComponent(dsRenderer = new DebugSpriteRenderer(this, "./assets/shaders/Static2D.vert", "./assets/shaders/Static2D.frag"));

		srenderer = new StaticRenderer("./assets/shaders/Static3D.vert", "./assets/shaders/Static3D.frag");
		GetKeyboard()->KeyPress.Add(this, &TestGame::KeyInput);
	}

	virtual void LoadContent(void)
	{
		cam = new Camera(GetGraphics()->GetWindow());
		cam->Move(Vector3(0.0f, 2.0f, -4.0f));

		map = StaticModel::FromFile("./assets/models/Sponza/sponza.obj");
		map->SetScale(0.03f); // If map is sponza
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(map);
	}

	virtual void Finalize(void)
	{
		GetKeyboard()->KeyPress.Remove(this, &TestGame::KeyInput);

		delete_s(srenderer);
		if (depthSprite) delete_s(depthSprite);
	}

	virtual void Update(float dt)
	{
		/* Update rotating light. */
		theta = modrads(theta += DEG2RAD * dt * 10);
		light = Vector3::FromRoll(theta);

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

			/* Operation will take a long time, so make sure it doesn't affect next frames delta. */
			SuppressNextUpdate();
		}
#endif
	}

	virtual void Render(float dt)
	{
		/* Render current scene, */
		srenderer->Begin(cam->GetView(), cam->GetProjection(), light, cam->GetPosition());
		srenderer->Render(map);
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

		/* Add debug average GRAM. */
		std::string gpuStr = "GPU: ";
		gpuStr += b2short_string(mem->GetAvrgGPURamUsage());
		dfRenderer->AddDebugString(gpuStr);

		/* Add debug frame buffer diagnostics. */
		if (depthSprite) dsRenderer->AddDebugTexture(depthSprite, Color::White, Vector2(0.1f));
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