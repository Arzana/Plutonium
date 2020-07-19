#include "TestGame.h"
#include "Core/Math/Shapes/Sphere.h"
#include <Core/Diagnostics/Profiler.h>
#include <imgui.h>

#define STRESS_TEST
#define USE_KNIGHT

using namespace Pu;

const bool enableTessellation = false;
const uint16 terrainSize = 10;

TestGame::TestGame(void)
	: Application(L"TestGame"),
	updateCam(false), firstRun(true), spawnToggle(false)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	GetInput().AnyMouseScrolled.Add(*this, &TestGame::OnAnyMouseScrolled);

	AddSystem(physics = new PhysicalWorld());
	physicsMat = physics->AddMaterial({ 1.0f, 0.5f, 0.8f });
}

bool TestGame::GpuPredicate(const PhysicalDevice & physicalDevice)
{
	const PhysicalDeviceFeatures &features = physicalDevice.GetSupportedFeatures();
	return features.VertexPipelineStoresAndAtomics;
}

void TestGame::EnableFeatures(const PhysicalDeviceFeatures & supported, PhysicalDeviceFeatures & enabeled)
{
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
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Update(float)
{
	if (spawnToggle) SpawnNPC();
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
		camFree->Move(-16.0f, 57.0f, -34.0f);
		camFree->Yaw = PI4;
		camFree->SetExposure(2.5f);

		lightMain = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightMain->SetDirection(PI4, PI4, 0.0f);
		lightMain->SetEnvironment(*skybox);
		lightMain->SetIntensity(4.0f);

		lightFill = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightFill->SetDirection(Quaternion::Create(PI, lightMain->GetUp()) * lightMain->GetOrientation());
		lightFill->SetIntensity(0.5f);
		lightFill->SetEnvironment(*skybox);

		renderer->SetSkybox(*skybox);
	}
	else if (!firstRun)
	{
		camFree->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		renderer->InitializeResources(cmd, *camFree);
		renderer->Begin(DeferredRenderer::SubpassTerrain);
		for (const TerrainChunk *chunk : terrain)
		{
			if (chunk->IsUsable()) renderer->Render(*chunk);
		}

#ifndef USE_KNIGHT
		if (playerModel->IsLoaded())
		{
			renderer->Begin(DeferredRenderer::SubpassBasicStaticGeometry);
			for (PhysicsHandle hnpc : npcs)
			{
				renderer->Render(*playerModel, physics->GetTransform(hnpc) * Matrix::CreateScalar(3.0f));
			}
		}
#else
		if (playerModel->IsLoaded())
		{
			renderer->Begin(DeferredRenderer::SubpassBasicMorphGeometry);
			for (PhysicsHandle hnpc : npcs)
			{
				const Matrix transform = physics->GetTransform(hnpc) * Matrix::CreateScalar(0.05f);
				renderer->Render(*playerModel, transform, 0, 1, 0.0f);
			}
		}
#endif
		renderer->Begin(DeferredRenderer::SubpassDirectionalLight);
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();

		physics->Visualize(*dbgRenderer, camFree->GetPosition(), dt);
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
		ImGui::EndMainMenuBar();
	}

	Profiler::Visualize();
}

void TestGame::SpawnNPC(void)
{
#ifdef STRESS_TEST
	const float x = random(10.0f, 63.0f * terrainSize - 10.0f);
	const float z = random(10.0f, 63.0f * terrainSize - 10.0f);
#else
	const float x = random(10.0f, 20.0f);
	const float z = random(10.0f, 20.0f);
#endif

	/* Simple sphere is good for height, but bad for width. */
	Sphere sphere{ Vector3{}, 1.5f };
	Collider collider{ AABB(-1.5f, -1.5f, -1.5f, 3.0f, 3.0f, 3.0f), CollisionShapes::Sphere, &sphere };

	/* Use the default physical material. */
	PhysicalObject obj{ Vector3(x, 50.0f, z), Quaternion{}, collider };
	obj.Properties = physicsMat;
	obj.State.Mass = 80.0f;
	obj.CoM = obj.P;

#ifdef USE_KNIGHT
	/* Moment of Inertia is that of a cylinder (using h = 1). */
	const float nonDomAxis = recip(12.0f) * obj.State.Mass * (3.0f * sqr(1.0f) + sqr(sphere.Radius));
	const float domAxis = 0.5f * obj.State.Mass * sqr(sphere.Radius);
	obj.MoI = Matrix3::CreateScalar(nonDomAxis, domAxis, nonDomAxis);
	obj.State.Cd = 0.82f;
#else
	/* Moment of Intertia of a solid sphere. */
	obj.MoI = Matrix3::CreateScalar((2.0f / 5.0f) * obj.State.Mass * sqr(sphere.Radius));
	obj.State.Cd = 0.5f;
#endif

	npcs.emplace_back(physics->AddKinematic(obj));
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
					TerrainChunk *chunk = new TerrainChunk(GetContent(), physics);
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