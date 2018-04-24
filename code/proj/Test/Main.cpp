#include <Game.h>
#include <Core\String.h>
#include <Graphics\Diagnostics\DebugTextRenderer.h>
#include <Graphics\Diagnostics\DebugSpriteRenderer.h>
#include <Graphics\Diagnostics\FrameInfo.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <Core\Math\Interpolation.h>

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
	Color lightClr;
	Vector3 lightDir;
	const char *dayState;
	StaticModel *map;

	/* Diagnostics. */
	FpsCounter *fps;
	MemoryCounter *mem;
	Texture *depthSprite;

	TestGame(void)
		: Game("TestGame"), depthSprite(nullptr), theta(0.0f)
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
		cam->Move(Vector3(0.0f, 5.0f, -3.0f));
		cam->Yaw = PI2;

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
		/* Update day night cycle. */
		UpdateDayState(dt);

		/* Update camera. */
		cam->Update(dt, GetKeyboard(), GetCursor());

		/* Update input. */
		if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
	}

	void UpdateDayState(float dt)
	{
		/* Define dusk and dawn thresholds. */
		constexpr float SUNSET = PI + 9.0f * DEG2RAD;
		constexpr float DUSK = PI + 18.0f * DEG2RAD;
		constexpr float DAWN = TAU - 18.0f * DEG2RAD;
		constexpr float SUNRISE = TAU - 9.0f * DEG2RAD;

		/* Update light orientation. */
		theta = modrads(theta += DEG2RAD * dt * 5.0f);
		lightDir = Vector3::FromRoll(theta);

		/* Update light color. */
		if (theta > 0.0f && theta < PI)
		{
			lightClr = Color::SunDay;
			dayState = "Day";
		}
		else if (theta > PI && theta < SUNSET)
		{
			lightClr = Color::Lerp(Color::SunDay, Color::SunDawn, ilerp(PI, SUNSET, theta));
			dayState = "Sunset";
		}
		else if (theta > SUNSET && theta < DUSK)
		{
			lightClr = Color::Lerp(Color::SunDawn, Color::Black, ilerp(SUNSET, DUSK, theta));
			dayState = "Dusk";
		}
		else if (theta > DUSK && theta < DAWN)
		{
			lightClr = Color::Black;
			dayState = "Night";
		}
		else if (theta > DAWN && theta < SUNRISE)
		{
			lightClr = Color::Lerp(Color::Black, Color::SunDawn, ilerp(DAWN, SUNRISE, theta));
			dayState = "Dawn";
		}
		else if (theta > SUNRISE && theta < TAU)
		{
			lightClr = Color::Lerp(Color::SunDawn, Color::SunDay, ilerp(SUNRISE, TAU, theta));
			dayState = "Sunrise";
		}
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
		srenderer->Begin(cam->GetView(), cam->GetProjection(), cam->GetPosition(), lightDir, lightClr);
		srenderer->Render(map);
		srenderer->End();

		/* Add debug light direction. */
		std::string lightStr = "Time: ";
		((lightStr += dayState) += ' ') += std::to_string(ipart(fmodf(6.0f + ::map(0.0f, 24.0f, theta, 0.0f, TAU), 24.0f)));
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