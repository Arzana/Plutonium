#include "TestGame.h"
#include "Core/Math/Shapes/Sphere.h"
#include <Core/Diagnostics/Profiler.h>
#include <imgui.h>

using namespace Pu;

const bool enableTessellation = false;
const uint16 terrainSize = 10;

TestGame::TestGame(void)
	: Application(L"TestGame"),
	updateCam(false), firstRun(true)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	GetInput().AnyMouseScrolled.Add(*this, &TestGame::OnAnyMouseScrolled);

	AddSystem(physics = new PhysicalWorld());
	physics->AddMaterial({ 1.0f, 0.2f, 0.8f });
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

	playerModel = &fetcher.FetchModel(L"{Models}knight.pum", *renderer, nullptr);
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
		camFree->Move(5.0f, 1.0f, -5.0f);
		camFree->SetExposure(2.5f);
		camFree->Yaw = TAU - PI4;

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
		renderer->BeginTerrain();
		for (const TerrainChunk *chunk : terrain)
		{
			if (chunk->IsUsable()) renderer->Render(*chunk);
		}

		renderer->BeginGeometry();
		renderer->BeginAdvanced();
		renderer->BeginMorph();
		if (playerModel->IsLoaded())
		{
			for (PhysicsHandle hnpc : npcs)
			{
				const Matrix transform = physics->GetTransform(hnpc) * Matrix::CreateScalar(0.05f);
				renderer->Render(*playerModel, transform, 0, 1, 0.0f);
			}
		}
		renderer->BeginLight();
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();

		physics->Visualize(*dbgRenderer, camFree->GetPosition());
		dbgRenderer->Render(cmd, *camFree);
	}

	Profiler::Visualize();
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
			const float x = random(10.0f, 40.0f);
			const float z = random(10.0f, 40.0f);

			Sphere sphere{ Vector3{}, 1.0f };
			Collider collider{ AABB(-1.0f, -1.0f, -1.0f, 2.0f, 2.0f, 2.0f), CollisionShapes::Sphere, &sphere };
			PhysicalObject obj{ Vector3(x, 30.0f, z), Quaternion{}, collider };
			obj.Properties = 0;
			obj.State.Mass = 1.0f;
			npcs.emplace_back(physics->AddKinematic(obj));
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