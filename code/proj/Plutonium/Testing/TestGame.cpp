#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <Streams/RuntimeConfig.h>
#include <imgui.h>

//#define STRESS_TEST
//#define USE_KNIGHT

using namespace Pu;

const uint16 terrainSize = 10;

TestGame::TestGame(void)
	: Application(L"TestGame"), updateCam(false), firstRun(true), spawnToggle(false), desiredFormat(nullptr)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	GetInput().AnyMouseScrolled.Add(*this, &TestGame::OnAnyMouseScrolled);
}

bool TestGame::GpuPredicate(const PhysicalDevice & physicalDevice)
{
	const PhysicalDeviceFeatures &features = physicalDevice.GetSupportedFeatures();
	return features.VertexPipelineStoresAndAtomics;
}

void TestGame::EnableFeatures(const PhysicalDeviceFeatures & supported, PhysicalDeviceFeatures & enabeled)
{
	const bool enableTessellation = RuntimeConfig::QueryBool(L"TessellationEnabled");

	enabeled.TessellationShader = supported.TessellationShader && enableTessellation;	// Optional for better terrain rendering.
	enabeled.WideLines = supported.WideLines;											// Debug renderer
	enabeled.FillModeNonSolid = supported.FillModeNonSolid;								// Easy wireframe mode
	enabeled.SamplerAnisotropy = supported.SamplerAnisotropy;							// Textures are loaded with 4 anisotropy by default
	enabeled.PipelineStatisticsQuery = supported.PipelineStatisticsQuery;				// Nice for performance testing, but optional
	enabeled.VertexPipelineStoresAndAtomics = true;										// Needed for imageLoad.
}

void TestGame::LoadContent(AssetFetcher & fetcher)
{
	renderer = new DeferredRenderer(fetcher, GetWindow());
	dbgRenderer = new DebugRenderer(GetWindow(), fetcher, &renderer->GetDepthBuffer(), 2.0f);
	GetWindow().SwapchainRecreated.Add(*this, &TestGame::OnSwapchainRecreated);

	AddSystem(world = new PhysicalWorld(*renderer));

	PhysicalProperties aluminum;
	aluminum.Density = 1.0f;
	aluminum.Mechanical.CoR = 0.2f;
	aluminum.Mechanical.CoFs = 1.15f;
	aluminum.Mechanical.CoFk = 1.4f;
	aluminum.Mechanical.CoFr = 0.001f;
	physicsMat = world->AddMaterial(aluminum);

	skybox = &fetcher.FetchSkybox(
		{
			L"{Textures}Skybox/right.jpg",
			L"{Textures}Skybox/left.jpg",
			L"{Textures}Skybox/top.jpg",
			L"{Textures}Skybox/bottom.jpg",
			L"{Textures}Skybox/front.jpg",
			L"{Textures}Skybox/back.jpg"
		});

#ifdef USE_KNIGHT
	playerModel = &fetcher.FetchModel(L"{Models}knight.pum", *renderer, nullptr);
#else
	playerModel = &fetcher.CreateModel(ShapeType::Sphere, *renderer, nullptr, L"{Textures}uv.png");
#endif

	rampModel = &fetcher.CreateModel(ShapeType::Box, *renderer);

	OBB obb{ Vector3(), Vector3(0.5f), Quaternion() };
	Collider collider{ obb };

	PhysicalObject obj{ Vector3{}, Quaternion::CreatePitch(PI4 * 0.5f), collider };
	obj.Properties = physicsMat;
	obj.Scale = Vector3(40.0f, 1.0f, 100.0f);
	world->AddStatic(obj, *rampModel, DeferredRenderer::SubpassBasicStaticGeometry);

	obj.P.Z = 90.0f;
	obj.Theta = Quaternion::CreatePitch(TAU - PI4 * 0.5f);
	world->AddStatic(obj, *rampModel, DeferredRenderer::SubpassBasicStaticGeometry);
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (camFree) delete camFree;
	if (lightMain) delete lightMain;
	if (lightFill) delete lightFill;
	for (TerrainChunk *chunk : terrain) delete chunk;
	if (renderer) delete renderer;
	if (dbgRenderer) delete dbgRenderer;

	fetcher.Release(*playerModel);
	fetcher.Release(*rampModel);
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Update(float)
{
	if (spawnToggle) SpawnNPC();
	if (desiredFormat)
	{
		GetWindow().SetColorSpace(*desiredFormat);
		desiredFormat = nullptr;
	}
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		renderer->InitializeCameraPool(*descPoolConst, 1);								// Camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);			// Light set
		descPoolConst->AddSet(DeferredRenderer::SubpassTerrain, 1, sqr(terrainSize));	// Terrain set

		camFree = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		camFree->Move(RuntimeConfig::QuerySingle(L"CamX"), RuntimeConfig::QuerySingle(L"CamY"), RuntimeConfig::QuerySingle(L"CamZ"));
		camFree->Pitch = RuntimeConfig::QuerySingle(L"CamPitch");
		camFree->Yaw = RuntimeConfig::QuerySingle(L"CamYaw");
		camFree->Roll = RuntimeConfig::QuerySingle(L"CamRoll");
		camFree->SetExposure(2.5f);

		lightMain = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightMain->SetRadiance(Color::SunDay());
		lightMain->SetDirection(PI4, PI4, 0.0f);
		lightMain->SetEnvironment(*skybox);
		lightMain->SetIntensity(4.0f);
		world->AddLight(*lightMain);

		lightFill = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightFill->SetDirection(Quaternion::Create(PI, lightMain->GetUp()) * lightMain->GetOrientation());
		lightMain->SetRadiance(Color::SunDay());
		lightFill->SetIntensity(0.5f);
		lightFill->SetEnvironment(*skybox);
		world->AddLight(*lightFill);

		renderer->SetSkybox(*skybox);
	}
	else if (!firstRun)
	{
		camFree->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		world->Render(*camFree, cmd);
		world->Visualize(*dbgRenderer, camFree->GetPosition(), dt);
		camFree->Visualize();
		dbgRenderer->AddTransform(Matrix{}, 2.0f, Vector3(0.0f, 20.0f, 0.0f));
		dbgRenderer->Render(cmd, *camFree);
	}

#ifndef _DEBUG
	if (ImGui::Begin("Counter"))
	{
		ImGui::Text("Kinematic Objects: %zu", npcs.size());
		ImGui::End();
	}
#endif

	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Text("FPS: %d", iround(recip(dt)));
		ImGui::Separator();
		if (ImGui::Button("Save Camera") && camFree)
		{
			RuntimeConfig::Set(L"CamX", camFree->GetPosition().X);
			RuntimeConfig::Set(L"CamY", camFree->GetPosition().Y);
			RuntimeConfig::Set(L"CamZ", camFree->GetPosition().Z);
			RuntimeConfig::Set(L"CamPitch", camFree->Pitch);
			RuntimeConfig::Set(L"CamYaw", camFree->Yaw);
			RuntimeConfig::Set(L"CamRoll", camFree->Roll);
		}

		const vector<SurfaceFormat> &formats = GetWindow().GetSupportedFormats();
		if (ImGui::BeginCombo("Formats", GetWindow().GetSwapchain().GetFormat().ToString().c_str()))
		{
			for (const SurfaceFormat &format : formats)
			{
				bool selected = false;
				ImGui::Selectable(format.ToString().c_str(), &selected);
				if (selected)
				{
					desiredFormat = &format;
					break;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::EndMainMenuBar();
	}

	Profiler::Visualize();
}

void TestGame::SpawnNPC(void)
{
#ifdef STRESS_TEST
	const float x = random(10.0f, 63.0f * terrainSize - 10.0f);
	const float y = 50.0f;
	const float z = random(10.0f, 63.0f * terrainSize - 10.0f);
#else
	const float x = 0.0f;
	const float y = 15.0f;
	const float z = -30.0f;
#endif

	/* Simple sphere is good for height, but bad for width. */
#ifdef USE_KNIGHT
	Sphere sphere{ 25.0f };
#else
	Sphere sphere{ 0.5f };
#endif
	Collider collider{ sphere };

	/* Use the default physical material. */
	PhysicalObject obj{ Vector3(x, y, z), Quaternion{}, collider };
	obj.Properties = physicsMat;
	obj.State.Mass = 80.0f;
	obj.CoM = obj.P;

#ifdef USE_KNIGHT
	/* Moment of Inertia is that of a cylinder (using h = 1). */
	const float nonDomAxis = recip(12.0f) * obj.State.Mass * (3.0f * sqr(1.0f) + sqr(sphere.Radius));
	const float domAxis = 0.5f * obj.State.Mass * sqr(sphere.Radius);
	obj.MoI = Matrix3::CreateScalar(nonDomAxis, domAxis, nonDomAxis);
	obj.State.Cd = 0.82f;
	obj.Scale = 0.05f;

	const uint32 subpass = DeferredRenderer::SubpassBasicMorphGeometry;
#else
	/* Moment of Intertia of a solid sphere. */
	obj.MoI = Matrix3::CreateScalar((2.0f / 5.0f) * obj.State.Mass * sqr(sphere.Radius));
	obj.State.Cd = 0.5f;
	obj.Scale = 3.0f;

	const uint32 subpass = DeferredRenderer::SubpassBasicStaticGeometry;
#endif

	npcs.emplace_back(world->AddKinematic(obj, *playerModel, subpass));
}

void TestGame::OnAnyMouseScrolled(const Pu::Mouse &, Pu::int16 value)
{
	if (camFree) camFree->MoveSpeed += value;
}

void TestGame::OnAnyKeyDown(const InputDevice & sender, const ButtonEventArgs &args)
{
	if (sender.Type == InputDeviceType::Keyboard)
	{
		if (args.Key == Keys::Escape) Exit();
		else if (args.Key == Keys::C)
		{
			if (updateCam = !updateCam) Mouse::HideAndLockCursor(GetWindow().GetNative());
			else Mouse::ShowAndFreeCursor();
		}
		else if (args.Key == Keys::G && descPoolConst && terrain.empty())
		{
			for (float z = 0; z < terrainSize; z++)
			{
				for (float x = 0; x < terrainSize; x++)
				{
					TerrainChunk *chunk = new TerrainChunk(GetContent(), world);
					chunk->Initialize(L"{Textures}uv.png", *descPoolConst, renderer->GetTerrainLayout(), noise, Vector2(x, z),
						{
							L"{Textures}Terrain/Water.jpg",
							L"{Textures}Terrain/Grass.jpg",
							L"{Textures}Terrain/Dirt.jpg",
							L"{Textures}Terrain/Snow.jpg"
						});

					terrain.emplace_back(chunk);
				}
			}
		}
		else if (args.Key == Keys::P)
		{
#ifdef STRESS_TEST
			spawnToggle = !spawnToggle;
#else
			SpawnNPC();
#endif
	}
}
	else if (sender.Type == InputDeviceType::GamePad)
	{
		if (args.Key == Keys::XBoxB) Exit();
	}
}

void TestGame::OnSwapchainRecreated(const GameWindow &, const SwapchainReCreatedEventArgs &)
{
	if (dbgRenderer) dbgRenderer->Reset(renderer->GetDepthBuffer());
}