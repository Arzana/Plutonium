#include "TestGame.h"
#include <Graphics\Models\Shapes.h>
#include <Graphics\Materials\MaterialBP.h>
#include <Graphics\Diagnostics\DebugSpriteRenderer.h>

//#define QUICK_MAP
#define BIG_MAP

TestGame::TestGame(void)
	: Game(_CRT_NAMEOF_RAW(TestGame)), sunAngle(TAU - PI4), 
	knight(nullptr), dayNightSpeed(0.0f)
{
	GetGraphics()->GetWindow()->SetMode(WindowMode::BorderlessFullscreen);
	GetCursor()->Disable();
}

void TestGame::Initialize(void)
{
	/* Add menu components. */
	AddComponent(hud = new HUD(this));
	AddComponent(loadScreen = new LoadScreen(this));

	/* Initialize renderers. */
	renderer = new DeferredRendererBP(this);
	sbrenderer = new SkyboxRenderer(GetGraphics());
	srenderer = new ShapeRenderer(GetGraphics());

	/* Bind keypress events. */
	GetKeyboard()->KeyPress.Add(this, &TestGame::SpecialKeyPress);

	/* Initialize camera. */
	cam = new Camera(GetGraphics());
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
	sun = new DirectionalLight(0.0f, 0.0f, sunAngle, Color(0.2f, 0.2f, 0.2f), Color::SunDay(), Color::White());
#if defined (QUICK_MAP)
	knight = new Knight(this, Vector3::Up() * 3.0f, 0.1f, PER_FIRE_WEIGHT * 4);
#elif !defined (BIG_MAP)
	Color fireColor = Color((byte)254, 211, 60);
	fires.push_back(new Fire(this, Vector3(-616.6f, 172.6f, 140.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(-616.6f, 172.6f, -220.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(490.6f, 172.6f, 140.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
	fires.push_back(new Fire(this, Vector3(490.6f, 172.6f, -220.3f), fireColor, MAP_SCALE, PER_FIRE_WEIGHT));
#else
	UpdateLoadPercentage(PER_FIRE_WEIGHT * 0.04f);
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
	delete_s(srenderer);
	delete_s(cam);
}

void TestGame::Update(float dt)
{
	/* Update visible menus. */
	loadScreen->Hide();
	hud->Show();

	/* Update scene. */
	UpdateDayState(dt);
	for (size_t i = 0; i < fires.size(); i++) fires.at(i)->Update(dt);
#if defined (QUICK_MAP)
	knight->Update(dt);
#endif

	/* Update camera. */
	cam->Update(dt, hud->HasFocus() ? nullptr : GetKeyboard(), GetCursor()->IsVisible() ? nullptr : GetCursor());

	/* Update input. */
	if (GetKeyboard()->IsKeyDown(Keys::Escape)) Exit();
}

void TestGame::Render(float dt)
{
	/* Render map and add sun light. */
	renderer->Add(map);
	renderer->Add(sun);

	/* Render knight if needed. */
#if defined(QUICK_MAP)
	renderer->Add(knight->object);
#endif

	/* Render all fires present in the scene. */
	for (size_t i = 0; i < fires.size(); i++)
	{
		renderer->Add(fires.at(i)->object);
		renderer->Add(fires.at(i)->light);
	}

	/* Render scene. */
	renderer->Render(cam);
	sbrenderer->Render(cam->GetView(), cam->GetProjection(), skybox);

	/* Render debugging shapes. */
#if defined (DEBUG)
	/* Render the direction indicator in the bottom left. */
	float offset = 25.0f;
	float rayLen = 0.01f;
	Vector2 pos(offset, GetGraphics()->GetWindow()->GetClientBounds().GetHeight() - offset);
	Matrix translation = Matrix::CreateTranslation(cam->ScreenToWorld(pos, cam->GetNear() + recip(rayLen)));
	Matrix scale = Matrix::CreateScalar(rayLen);

	srenderer->AddMatrix(translation * scale);
	srenderer->Render(cam->GetView(), cam->GetProjection());
#endif
}

void TestGame::UpdateDayState(float dt)
{
	/* Define dusk and dawn thresholds. */
	constexpr float SUNSET = PI + 9.0f * DEG2RAD;
	constexpr float DUSK = PI + 18.0f * DEG2RAD;
	constexpr float DAWN = TAU - 18.0f * DEG2RAD;
	constexpr float SUNRISE = TAU - 9.0f * DEG2RAD;

	/* Update light orientation. */
	sunAngle = modrads(sunAngle += DEG2RAD * dt * dayNightSpeed);
	sun->SetDirection(0.0f, 0.0f, sunAngle);

	/* Update light color. */
	sun->Diffuse = Color::SunDay();
}

void TestGame::SpecialKeyPress(WindowHandler, const KeyEventArgs args)
{
	if (hud->HasFocus()) return;
	RenderType desired = RenderType::Normal;

	/* Check if pressed key is debug render toggle key. */
	if (args.Key == Keys::D1 && args.Action == KeyState::Down) desired = RenderType::Wireframe;
	if (args.Key == Keys::D2 && args.Action == KeyState::Down) desired = RenderType::WorldNormals;
	if (args.Key == Keys::D3 && args.Action == KeyState::Down) desired = RenderType::Albedo;
	if (args.Key == Keys::D4 && args.Action == KeyState::Down) desired = RenderType::Lighting;
	if (args.Key == Keys::D5 && args.Action == KeyState::Down) desired = RenderType::Shadows;

	/* Check for changes in render mode. */
	if (desired != RenderType::Normal) renderer->DisplayType = renderer->DisplayType == desired ? RenderType::Normal : desired;

	/* Check for mouse release. */
	if (args.Key == Keys::M && _CrtEnumCheckFlag(args.Mods, KeyMods::Shift) && args.Action == KeyState::Down)
	{
		if (GetCursor()->IsVisible()) GetCursor()->Disable();
		else GetCursor()->Show();
	}
}