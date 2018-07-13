#include "TestGame.h"
#include "Graphics\Models\Shapes.h"
#include "Graphics\Materials\MaterialBP.h"

#define QUICK_MAP
#define SHDR_PATH(name)		"./assets/shaders/" name

TestGame::TestGame(void)
	: Game(_CRT_NAMEOF_RAW(TestGame)), dayState("<NULL>"),
	sunAngle(0.0f), renderMode(DebuggableValues::None)
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
	srenderer = new StaticRenderer(GetGraphics(), SHDR_PATH("Static3D.vert"), SHDR_PATH("Static3D.frag"));
	drenderer = new DynamicRenderer(SHDR_PATH("Dynamic3D.vert"), SHDR_PATH("Dynamic3D.frag"));
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
	static constexpr int SKYBOX_WEIGHT = 5;
	static constexpr int PER_FIRE_WEIGHT = 10;

	/* Load static map. */
#if defined (QUICK_MAP)
	static constexpr int MAP_WEIGHT = 54;
	constexpr float MAP_SCALE = 1.0f;
	map = new StaticObject(this, "models/Ruin/ruin2_walled.obj", MAP_WEIGHT);
#else
	static constexpr int MAP_WEIGHT = 55;
	constexpr float MAP_SCALE = 0.03f;
	map = new StaticObject(this, "models/Sponza/sponza.obj", MAP_WEIGHT);
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
	sun = new DirectionalLight(Vector3::FromRoll(sunAngle), Color(0.2f, 0.2f, 0.2f), Color::SunDay, Color::White);
#if defined (QUICK_MAP)
	UpdateLoadPercentage(PER_FIRE_WEIGHT * 4 * 0.01f);

	GetLoader()->LoadTexture("textures/uv.png", Callback<Texture>([&](const AssetLoader*, Texture *texture)
	{
		MaterialBP *mat = new MaterialBP("VisualizerMaterial", GetLoader(), nullptr, texture, nullptr, nullptr, nullptr);

		Mesh *mesh = new Mesh("VisualizerMesh");
		ShapeCreator::MakeSphere(mesh, 64, 64);
		mesh->Finalize(GetGraphics()->GetWindow());

		visualizer = new StaticObject(this, new StaticModel(mat, mesh), 2);
		visualizer->Move(Vector3::Up * 5.0f);
}));
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
	delete_s(visualizer);
#endif
	delete_s(sun);
	for (size_t i = 0; i < fires.size(); i++) delete_s(fires.at(i));
	fires.clear();
	GetLoader()->Unload(skybox);
}

void TestGame::Finalize(void)
{
	delete_s(srenderer);
	delete_s(drenderer);
	delete_s(sbrenderer);
	delete_s(dmrenderer);
	delete_s(cam);
}

void TestGame::Update(float dt)
{
	/* Update visible menus. */
	loadScreen->Hide();
	hud->Show();

	/* Update lighting. */
	UpdateDayState(dt);
	for (size_t i = 0; i < fires.size(); i++) fires.at(i)->Update(dt);

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
		/* Render dynamic objects. */
		drenderer->Begin(cam->GetView(), cam->GetProjection(), sun->Direction);
		for (size_t i = 0; i < fires.size(); i++) drenderer->Render(fires.at(i)->object);
		drenderer->End();

		/* Setup lighting for render. */
#if defined (QUICK_MAP)
		const PointLight empty(Vector3::Zero, Color::Black, 1.0f, 1.0f, 1.0f);
		const PointLight *lights[4] = { &empty, &empty, &empty, &empty };
#else
		const PointLight *lights[4] = { fires.at(0)->light, fires.at(1)->light, fires.at(2)->light, fires.at(3)->light };
#endif

		/* Render static map. */
		srenderer->Begin(cam->GetView(), cam->GetProjection(), cam->GetPosition(), sun, lights);
		srenderer->Render(map);
#if defined (QUICK_MAP)
		srenderer->Render(visualizer);
#endif
		srenderer->End();

		/* Render skybox. */
		sbrenderer->Render(cam->GetView(), cam->GetProjection(), skybox);
	}
	else
	{
		/* Render debug scene. */
		dmrenderer->AddModel(map);
		dmrenderer->AddLight(sun);
#if defined (QUICK_MAP)
		dmrenderer->AddModel(visualizer);
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
#ifndef QUICK_MAP
	sunAngle = modrads(sunAngle += DEG2RAD * dt * 25.0f);
#else
	sunAngle = DEG2RAD * 45.0f;
#endif
	sun->Direction = Vector3::FromRoll(sunAngle);

	/* Update light color. */
	if (sunAngle >= 0.0f && sunAngle < PI)
	{
		sun->Diffuse = Color::SunDay;
		dayState = "Day";
	}
	else if (sunAngle > PI && sunAngle < SUNSET)
	{
		sun->Diffuse = Color::Lerp(Color::SunDay, Color::SunDawn, PI, SUNSET, sunAngle);
		dayState = "Sunset";
	}
	else if (sunAngle > SUNSET && sunAngle < DUSK)
	{
		sun->Diffuse = Color::Lerp(Color::SunDawn, Color::Black, SUNSET, DUSK, sunAngle);
		dayState = "Dusk";
	}
	else if (sunAngle > DUSK && sunAngle < DAWN)
	{
		sun->Diffuse = Color::Black;
		dayState = "Night";
	}
	else if (sunAngle > DAWN && sunAngle < SUNRISE)
	{
		sun->Diffuse = Color::Lerp(Color::Black, Color::SunDawn, DAWN, SUNRISE, sunAngle);
		dayState = "Dawn";
	}
	else if (sunAngle > SUNRISE && sunAngle < TAU)
	{
		sun->Diffuse = Color::Lerp(Color::SunDawn, Color::SunDay, SUNRISE, TAU, sunAngle);
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