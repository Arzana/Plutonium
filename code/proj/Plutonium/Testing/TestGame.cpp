#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <Streams/RuntimeConfig.h>
#include <imgui.h>

using namespace Pu;

const uint16 terrainSize = 10;

TestGame::TestGame(void)
	: Application(L"TestGame"), updateCam(false), firstRun(true), spawnToggle(false),
	desiredFormat(nullptr), vsynchMode(-1), updateRenderer(false), showAssets(false)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	GetInput().AnyMouseScrolled.Add(*this, &TestGame::OnAnyMouseScrolled);
}

void TestGame::EnableFeatures(const PhysicalDeviceFeatures & supported, PhysicalDeviceFeatures & enabeled, vector<const char*> &extensions)
{
	enabeled.TessellationShader = supported.TessellationShader;				// Optional for better terrain rendering.
	enabeled.WideLines = supported.WideLines;								// Debug renderer
	enabeled.FillModeNonSolid = supported.FillModeNonSolid;					// Easy wireframe mode
	enabeled.SamplerAnisotropy = supported.SamplerAnisotropy;				// Textures are loaded with 4 anisotropy by default
	enabeled.PipelineStatisticsQuery = supported.PipelineStatisticsQuery;	// Nice for performance testing, but optional

	extensions.emplace_back("VK_EXT_line_rasterization");					// Smoother debug lines.
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
	world->AddMaterial(aluminum);

	skybox = &fetcher.FetchSkybox(
		{
			L"{Textures}Skybox/right.jpg",
			L"{Textures}Skybox/left.jpg",
			L"{Textures}Skybox/top.jpg",
			L"{Textures}Skybox/bottom.jpg",
			L"{Textures}Skybox/front.jpg",
			L"{Textures}Skybox/back.jpg"
		});
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (camFree) delete camFree;
	if (lightMain) delete lightMain;
	if (lightFill) delete lightFill;
	for (TerrainChunk *chunk : terrain) delete chunk;
	if (renderer) delete renderer;
	if (dbgRenderer) delete dbgRenderer;

	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Update(float)
{
	if (desiredFormat)
	{
		GetWindow().SetColorSpace(*desiredFormat);
		desiredFormat = nullptr;
	}

	if (updateRenderer)
	{
		updateRenderer = false;
		renderer->UpdateConfigurableProperties();
	}

	if (vsynchMode == 0)
	{
		vsynchMode = -1;
		IsFixedTimeStep = false;
		GetWindow().SetPresentMode(PresentMode::Immediate);
	}
	else if (vsynchMode == 1)
	{
		vsynchMode = -1;
		IsFixedTimeStep = true;
		GetWindow().SetPresentMode(PresentMode::MailBox);
	}
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Text("FPS: %d", iround(recip(dt)));

		if (ImGui::BeginMenu("Settings"))
		{
			const vector<SurfaceFormat> &formats = GetWindow().GetSupportedFormats();
			if (ImGui::BeginCombo("SurfaceFormat", to_string(GetWindow().GetSwapchain().GetColorSpace())))
			{
				for (const SurfaceFormat &format : formats)
				{
					bool selected = false;
					ImGui::Selectable(to_string(format.ColorSpace), &selected);
					if (selected) desiredFormat = &format;
				}

				ImGui::EndCombo();
			}

			bool selected = GetWindow().GetSwapchain().GetPresentMode() == PresentMode::MailBox;
			if (ImGui::Checkbox("VSync", &selected)) vsynchMode = selected;

			selected = RuntimeConfig::QueryBool(L"FXAAEnabled");
			if (ImGui::Checkbox("FXAA", &selected))
			{
				RuntimeConfig::Set(L"FXAAEnabled", selected);
				updateRenderer = true;
			}

			selected = RuntimeConfig::QueryBool(L"TessellationEnabled");
			if (ImGui::Checkbox("Tessellation", &selected))
			{
				RuntimeConfig::Set(L"TessellationEnabled", selected);
				updateRenderer = true;

				for (TerrainChunk *chunk : terrain) delete chunk;
				terrain.clear();
			}

			ImGui::MenuItem("Profiler", nullptr, &showProfiler);
			ImGui::MenuItem("Physics", nullptr, &showPhysics);
			ImGui::MenuItem("Camera", nullptr, &showCamOpt);
			ImGui::MenuItem("Assets", nullptr, &showAssets);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (renderer->IsUsable())
	{
		if (firstRun && skybox->IsUsable())
		{
			firstRun = false;

			descPoolConst = new DescriptorPool(renderer->GetRenderpass());
			renderer->InitializeCameraPool(*descPoolConst, 1);								// Camera sets
			descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);			// Light set
			descPoolConst->AddSet(DeferredRenderer::SubpassTerrain, 1, sqr(terrainSize));	// Terrain set

			camFree = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
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
			descPoolConst->Update(cmd, PipelineStageFlags::VertexShader);
			world->Render(*camFree, cmd);

			if (showPhysics) world->Visualize(*dbgRenderer, camFree->GetPosition());
			if (showCamOpt) camFree->Visualize();

			const float rayLen = 0.01f;
			const Vector3 p = camFree->NDCToWorld(Vector3{ 0.9f, 0.9f, camFree->GetNear() + rayLen });
			dbgRenderer->AddTransform(Matrix{}, rayLen, p);
			dbgRenderer->Render(cmd, *camFree);
		}
	}

#ifndef _DEBUG
	if (ImGui::Begin("Counter"))
	{
		ImGui::Text("Kinematic Objects: %zu", npcs.size());
		ImGui::End();
	}
#endif

	if (showProfiler) Profiler::Visualize();
	if (showAssets) GetContent().Visualize();
}

void TestGame::OnAnyMouseScrolled(const Mouse&, int16 value)
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