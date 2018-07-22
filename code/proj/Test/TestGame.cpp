#include "TestGame.h"
#include <Graphics\Models\Shapes.h>
#include <Graphics\Materials\MaterialBP.h>

#define QUICK_MAP
//#define BIG_MAP
//#define DISABLE_VSYNC

TestGame::TestGame(void)
	: Game(_CRT_NAMEOF_RAW(TestGame)), dayState("<NULL>"),
	sunAngle(0.0f), renderMode(DebuggableValues::None), knight(nullptr)
{
#if defined (DISABLE_VSYNC)
	FixedTimeStep = false;
	GetGraphics()->GetWindow()->SetMode(VSyncMode::Disable);
#endif

	GetGraphics()->GetWindow()->SetMode(WindowMode::BorderlessFullscreen);
	GetCursor()->Disable();
}

void TestGame::Initialize(void)
{
	/* Add menu components. */
	AddComponent(hud = new HUD(this));
	AddComponent(loadScreen = new LoadScreen(this));

	/* Initialize renderers. */
	renderer = new DeferredRendererBP(GetGraphics());
	sbrenderer = new SkyboxRenderer(GetGraphics());
	dmrenderer = new DebugMeshRenderer(GetGraphics());

	/* Bind keypress events. */
	GetKeyboard()->KeyPress.Add(this, &TestGame::SpecialKeyPress);

	/* Initialize camera. */
	cam = new Camera(GetGraphics()->GetWindow());
	cam->Move(Vector3(0.0f, 5.0f, -3.0f));
	cam->Yaw = PI2;
}

void TestGame::LoadContent(void)
{
	/* Define load weights. */
	constexpr int MAP_WEIGHT = 55;
	static constexpr int SKYBOX_WEIGHT = 5;
	static constexpr int PER_FIRE_WEIGHT = 10;

	/* Load static map. */
#if defined (QUICK_MAP)
	constexpr float MAP_SCALE = 1.0f;
	map = new StaticObject(this, "models/Ruin/ruin2_walled.obj", MAP_WEIGHT);
#else
#if defined (BIG_MAP)
	constexpr float MAP_SCALE = 1.0f;
	map = new StaticObject(this, "models/SanMiguel/san-miguel-low-poly.obj", MAP_WEIGHT);
#else
	constexpr float MAP_SCALE = 0.03f;
	map = new StaticObject(this, "models/Sponza/sponza.obj", MAP_WEIGHT);
#endif
#endif
	map->SetScale(MAP_SCALE);

	/* Load skybox. */
	const char *skyboxPaths[] =
	{
		"models/Skybox/right.jpg",
		"models/Skybox/left.jpg",
		"models/Skybox/top.jpg",
		"models/Skybox/bottom.jpg",
		"models/Skybox/front.jpg",
		"models/Skybox/back.jpg"
	};
	GetLoader()->LoadTexture(skyboxPaths, Callback<Texture>([&](const AssetLoader*, Texture *result)
	{
		skybox = result;
		UpdateLoadPercentage(SKYBOX_WEIGHT * 0.01f);
	}));

	/* Setup lighting. */
	sun = new DirectionalLight(Vector3::FromRoll(sunAngle), Color(0.2f, 0.2f, 0.2f), Color::SunDay(), Color::White());
#if defined (QUICK_MAP)
	knight = new Knight(this, Vector3::Up() * 3.0f, 0.1f, PER_FIRE_WEIGHT * 4);
#else
	Color fireColor = Color((byte)254, 211, 60);
	fires.push_back(new Fire(this, Vector3(-616.6f, 172.6f, 140.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(-616.6f, 172.6f, -220.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(490.6f, 172.6f, 140.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(490.6f, 172.6f, -220.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
#endif
}

void TestGame::UnLoadContent(void)
{
	delete_s(map);
#if defined (QUICK_MAP)
	delete_s(knight);
#endif
	delete_s(sun);
	for (size_t i = 0; i < fires.size(); i++) delete_s(fires.at(i));
	fires.clear();
	GetLoader()->Unload(skybox);
}

void TestGame::Finalize(void)
{
	delete_s(renderer);
	delete_s(sbrenderer);
	delete_s(dmrenderer);
	delete_s(cam);
}

void TestGame::Update(float dt)
{
	/* Update visible menus. */
	loadScreen->Hide();
	hud->Show();

	/* Update scene. */
	UpdateDayState(dt);
#if defined (QUICK_MAP)
	knight->Update(dt);
#else
	for (size_t i = 0; i < fires.size(); i++) fires.at(i)->Update(dt);
#endif

	/* Update camera. */
	cam->Update(dt, hud->HasFocus() ? nullptr : GetKeyboard(), GetCursor()->IsVisible() ? nullptr : GetCursor());

	/* Update input. */
	if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
}

void TestGame::Render(float dt)
{
	/* Render scene normally. */
	if (renderMode == DebuggableValues::None)
	{
		/* Render static map. */
		renderer->Add(map);
		renderer->Add(sun);

#if defined(QUICK_MAP)
		renderer->Add(knight->object);
#else
		for (size_t i = 0; i < 4; i++)
		{
			renderer->Add(fires.at(i)->object);
			renderer->Add(fires.at(i)->light);
		}
#endif

		renderer->Render(cam->GetProjection(), cam->GetView(), cam->GetPosition());

		/* Render skybox. */
		sbrenderer->Render(cam->GetView(), cam->GetProjection(), skybox);
	}
	else
	{
		/* Render debug scene. */
		dmrenderer->AddModel(map);
		dmrenderer->AddLight(sun);
#if defined (QUICK_MAP)
		dmrenderer->AddModel(knight->object);
#else
		for (size_t i = 0; i < fires.size(); i++)
		{
			dmrenderer->AddModel(fires.at(i)->object);
			dmrenderer->AddLight(fires.at(i)->light);
		}
#endif
		dmrenderer->Render(cam->GetView(), cam->GetProjection(), cam->GetPosition());
	}
}

void TestGame::UpdateDayState(float dt)
{
	/* Define dusk and dawn thresholds. */
	constexpr float SUNSET = PI + 9.0f * DEG2RAD;
	constexpr float DUSK = PI + 18.0f * DEG2RAD;
	constexpr float DAWN = TAU - 18.0f * DEG2RAD;
	constexpr float SUNRISE = TAU - 9.0f * DEG2RAD;

	/* Update light orientation. */
	if (enableDayNight) sunAngle = modrads(sunAngle += DEG2RAD * dt * 25.0f);
	else sunAngle = DEG2RAD * 45.0f;
	sun->Direction = Vector3::FromRoll(sunAngle);

	/* Update light color. */
	if (sunAngle >= 0.0f && sunAngle < PI)
	{
		sun->Diffuse = Color::SunDay();
		dayState = "Day";
	}
	else if (sunAngle > PI && sunAngle < SUNSET)
	{
		sun->Diffuse = Color::Lerp(Color::SunDay(), Color::SunDawn(), PI, SUNSET, sunAngle);
		dayState = "Sunset";
	}
	else if (sunAngle > SUNSET && sunAngle < DUSK)
	{
		sun->Diffuse = Color::Lerp(Color::SunDawn(), Color::Black(), SUNSET, DUSK, sunAngle);
		dayState = "Dusk";
	}
	else if (sunAngle > DUSK && sunAngle < DAWN)
	{
		sun->Diffuse = Color::Black();
		dayState = "Night";
	}
	else if (sunAngle > DAWN && sunAngle < SUNRISE)
	{
		sun->Diffuse = Color::Lerp(Color::Black(), Color::SunDawn(), DAWN, SUNRISE, sunAngle);
		dayState = "Dawn";
	}
	else if (sunAngle > SUNRISE && sunAngle < TAU)
	{
		sun->Diffuse = Color::Lerp(Color::SunDawn(), Color::SunDay(), SUNRISE, TAU, sunAngle);
		dayState = "Sunrise";
	}
}

void TestGame::SpecialKeyPress(WindowHandler, const KeyEventArgs args)
{
	if (hud->HasFocus()) return;
	DebuggableValues desired = DebuggableValues::None;

	/* Check if pressed key is debug render toggle key. */
	if (args.Key == Keys::D1 && args.Action == KeyState::Down) desired = DebuggableValues::Wireframe;
	if (args.Key == Keys::D2 && args.Action == KeyState::Down) desired = DebuggableValues::Normals;
	if (args.Key == Keys::D3 && args.Action == KeyState::Down) desired = DebuggableValues::Unlit;
	if (args.Key == Keys::D4 && args.Action == KeyState::Down) desired = DebuggableValues::Lighting;

	/* Check for changes in render mode. */
	if (desired != DebuggableValues::None)
	{
		renderMode = renderMode == desired ? DebuggableValues::None : desired;
		dmrenderer->SetMode(renderMode);
	}

	/* Check for mouse release. */
	if (args.Key == Keys::M && _CrtEnumCheckFlag(args.Mods, KeyMods::Shift) && args.Action == KeyState::Down)
	{
		if (GetCursor()->IsVisible()) GetCursor()->Disable();
		else GetCursor()->Show();
	}
}