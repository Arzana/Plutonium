#include <Game.h>
#include <Core\String.h>
#include <Core\Math\Interpolation.h>
#include <Graphics\Diagnostics\DebugTextRenderer.h>
#include <Graphics\Diagnostics\DebugSpriteRenderer.h>
#include <Graphics\Text\TextRenderer.h>
#include <Graphics\Rendering\StaticRenderer.h>
#include <Graphics\Rendering\DynamicRenderer.h>
#include <Components\Camera.h>
#include <Components\MemoryCounter.h>
#include <Components\FpsCounter.h>
#include <GameLogic\StaticObject.h>
#include "Fire.h"

using namespace Plutonium;

struct TestGame
	: public Game
{
	/* Renderers. */
	DebugFontRenderer *dfRenderer;
	DebugSpriteRenderer *dsRenderer;
	FontRenderer *fRenderer;
	StaticRenderer *srenderer;
	DynamicRenderer *drenderer;
	Camera *cam;

	/* Scene */
	static constexpr float scale = 0.03f;	// If map is sponza
	float theta;
	DirectionalLight *sun;
	Fire *fires[4];
	const char *dayState;
	StaticObject *map;

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

		fRenderer = new FontRenderer(GetGraphics(), "./assets/fonts/OpenSans-Regular.ttf", "./assets/shaders/Text2D.vert", "./assets/shaders/Text2D.frag");
		srenderer = new StaticRenderer("./assets/shaders/Static3D.vert", "./assets/shaders/Static3D.frag");
		drenderer = new DynamicRenderer("./assets/shaders/Dynamic3D.vert", "./assets/shaders/Dynamic3D.frag");
	}

	virtual void LoadContent(void)
	{
		/* Setup camera for sponza. */
		cam = new Camera(GetGraphics()->GetWindow());
		cam->Move(Vector3(0.0f, 5.0f, -3.0f));
		cam->Yaw = PI2;

		/* Load static assets. */
		map = new StaticObject(this, "models/Sponza/sponza.obj", 100);
		map->SetScale(scale);

		/* Setup lighting. */
		sun = new DirectionalLight(Vector3::FromRoll(theta), Color(0.2f, 0.2f, 0.2f), Color::SunDay, Color::White);
		fires[0] = new Fire(GetGraphics()->GetWindow(), "./assets/models/Fire/fire.md2", "fire.png", Vector3(-616.6f, 172.6f, 140.3f), scale);
		fires[1] = new Fire(GetGraphics()->GetWindow(), "./assets/models/Fire/fire.md2", "fire.png", Vector3(-616.6f, 172.6f, -220.3f), scale);
		fires[2] = new Fire(GetGraphics()->GetWindow(), "./assets/models/Fire/fire.md2", "fire.png", Vector3(490.6f, 172.6f, 140.3f), scale);
		fires[3] = new Fire(GetGraphics()->GetWindow(), "./assets/models/Fire/fire.md2", "fire.png", Vector3(490.6f, 172.6f, -220.3f), scale);
	}

	virtual void UnLoadContent(void)
	{
		delete_s(cam);
		delete_s(sun);
		for (size_t i = 0; i < 4; i++) delete_s(fires[i]);
		delete_s(map);
	}

	virtual void Finalize(void)
	{
		delete_s(srenderer);
		delete_s(drenderer);
		if (depthSprite) delete_s(depthSprite);
	}

	virtual void Update(float dt)
	{
		/* Update lights. */
		UpdateDayState(dt);
		for (size_t i = 0; i < 4; i++) fires[i]->Update(dt);

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
		theta = modrads(theta += DEG2RAD * dt * 25.0f);
		sun->Direction = Vector3::FromRoll(theta);

		/* Update light color. */
		if (theta > 0.0f && theta < PI)
		{
			sun->Diffuse = Color::SunDay;
			dayState = "Day";
		}
		else if (theta > PI && theta < SUNSET)
		{
			sun->Diffuse = Color::Lerp(Color::SunDay, Color::SunDawn, PI, SUNSET, theta);
			dayState = "Sunset";
		}
		else if (theta > SUNSET && theta < DUSK)
		{
			sun->Diffuse = Color::Lerp(Color::SunDawn, Color::Black, SUNSET, DUSK, theta);
			dayState = "Dusk";
		}
		else if (theta > DUSK && theta < DAWN)
		{
			sun->Diffuse = Color::Black;
			dayState = "Night";
		}
		else if (theta > DAWN && theta < SUNRISE)
		{
			sun->Diffuse = Color::Lerp(Color::Black, Color::SunDawn, DAWN, SUNRISE, theta);
			dayState = "Dawn";
		}
		else if (theta > SUNRISE && theta < TAU)
		{
			sun->Diffuse = Color::Lerp(Color::SunDawn, Color::SunDay, SUNRISE, TAU, theta);
			dayState = "Sunrise";
		}
	}

	virtual void Render(float dt)
	{
		/* Render light sources. */
		drenderer->Begin(cam->GetView(), cam->GetProjection(), Vector3::Zero);
		for (size_t i = 0; i < 4; i++) drenderer->Render(fires[i]->model);
		drenderer->End();

		/* Render current scene, */
		const PointLight *lights[4] = { fires[0]->light, fires[1]->light, fires[2]->light, fires[3]->light };
		srenderer->Begin(cam->GetView(), cam->GetProjection(), cam->GetPosition(), sun, lights);
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

	virtual void RenderLoad(float dt, int percentage)
	{
		/* Update dot count at the end of the loading state to indicate that the game hasen't frozen. */
		static float accum = 0.0f;
		static int dotCnt = 0;
		if ((accum += dt) > 1.0f)
		{
			accum = 0.0f;
			if (++dotCnt > 3) dotCnt = 0;
		}

		/* Create loading state. */
		std::string loadStr = GetLoader()->GetState();
		(loadStr += '(') += std::to_string(percentage) += "%)";
		for (size_t i = 0; i < dotCnt; i++) loadStr += '.';

		/* Render loading state at the center of the screen. */
		Vector2 drawPos = GetGraphics()->GetWindow()->GetClientBounds().GetCenter();
		fRenderer->AddString(drawPos, loadStr.c_str());
		fRenderer->Render();
	}
};

int main(int argc, char **argv)
{
	TestGame *game = new TestGame();
	game->Run();
	delete_s(game);

#if defined(DEBUG)
	_CrtPressAnyKeyToContinue();
#endif
	return 0;
}