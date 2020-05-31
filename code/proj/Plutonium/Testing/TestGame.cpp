#include "TestGame.h"
#include "Core/Math/Shapes/Sphere.h"
#include <Core/Diagnostics/Profiler.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame"),
	updateCam(false), firstRun(true)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
}

bool TestGame::GpuPredicate(const PhysicalDevice & physicalDevice)
{
	const PhysicalDeviceFeatures &features = physicalDevice.GetSupportedFeatures();
	return features.GeometryShader && features.TessellationShader;
}

void TestGame::EnableFeatures(const Pu::PhysicalDeviceFeatures & supported, Pu::PhysicalDeviceFeatures & enabeled)
{
	enabeled.GeometryShader = true;		// Needed for the light probe renderer.
	enabeled.TessellationShader = true;	// Needed for terrain rendering.

	if (supported.WideLines) enabeled.WideLines = true;					// Debug renderer
	if (supported.FillModeNonSolid) enabeled.FillModeNonSolid = true;	// Easy wireframe mode
	if (supported.SamplerAnisotropy) enabeled.SamplerAnisotropy = true;	// Textures are loaded with 4 anisotropy by default
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

	terrain = new TerrainChunk(fetcher);
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (camFree) delete camFree;
	if (lightMain) delete lightMain;
	if (lightFill) delete lightFill;
	if (terrain) delete terrain;
	if (renderer) delete renderer;
	if (dbgRenderer) delete dbgRenderer;

	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		renderer->InitializeCameraPool(*descPoolConst, 1);						// Camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);	// Light set
		descPoolConst->AddSet(DeferredRenderer::SubpassTerrain, 1, 1);			// Terrain set

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
		if (terrain->IsUsable()) renderer->Render(*terrain);
		renderer->BeginGeometry();
		renderer->BeginAdvanced();
		renderer->BeginMorph();
		renderer->BeginLight();
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();

		dbgRenderer->Render(cmd, *camFree);
	}

	Profiler::Visualize();
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
		else if (args.Key == Keys::NumAdd) camFree->MoveSpeed++;
		else if (args.Key == Keys::NumSubtract) camFree->MoveSpeed--;
		else if (args.Key == Keys::G && terrain && !terrain->IsGenerated())
		{
			terrain->Initialize(L"{Textures}uv.png", *descPoolConst, renderer->GetTerrainLayout(), noise, 
				{
					L"{Textures}Terrain/Water.jpg",
					L"{Textures}Terrain/Grass.jpg",
					L"{Textures}Terrain/Dirt.jpg",
					L"{Textures}Terrain/Snow.jpg"
				});
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