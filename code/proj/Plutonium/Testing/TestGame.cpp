#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <Streams/RuntimeConfig.h>
#include <imgui.h>

enum class NPC_t
{
	Knight,
	Sphere,
	Box
};

using namespace Pu;

const uint16 terrainSize = 10;
const NPC_t npc_type = NPC_t::Box;

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

	if constexpr  (npc_type == NPC_t::Knight)
	{
		playerModel = &fetcher.FetchModel(L"{Models}knight.pum", *renderer, nullptr);
	}
	else playerModel = &fetcher.CreateModel(npc_type == NPC_t::Sphere ? ShapeType::Sphere : ShapeType::Box, *renderer, nullptr, L"{Textures}uv.png");

	rampModel = &fetcher.CreateModel(ShapeType::Box, *renderer);
	OBB obb{ Vector3(), Vector3(0.5f), Quaternion() };
	Collider collider{ obb };

	PhysicalObject obj{ Vector3{}, Quaternion::CreatePitch(PI8), collider };
	obj.Properties = physicsMat;
	obj.Scale = Vector3(40.0f, 1.0f, 100.0f);
	world->AddStatic(obj, *rampModel, DeferredRenderer::SubpassBasicStaticGeometry);

	obj.P.Z = 90.0f;
	obj.Theta = Quaternion::CreatePitch(TAU - PI8);
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

	/* Use the default physical material. */
	PhysicalObject obj{ Vector3{x, y, z}, Quaternion{} };
	obj.Properties = physicsMat;
	obj.State.Mass = 80.0f;
	obj.CoM = obj.P;

	uint32 subpass;
	if constexpr (npc_type == NPC_t::Knight)
	{
		/* Sphere is the only one that properly works now, but it's not good for the knight. */
		Sphere sphere{ 25.0f };
		obj.Collider = Collider{ sphere };

		/* Moment of Inertia is that of a cylinder (using h = 1). */
		const float nonDomAxis = recip(12.0f) * obj.State.Mass * (3.0f * sqr(1.0f) + sqr(sphere.Radius));
		const float domAxis = 0.5f * obj.State.Mass * sqr(sphere.Radius);
		obj.MoI = Matrix3::CreateScalar(nonDomAxis, domAxis, nonDomAxis);
		obj.State.Cd = 0.82f;
		obj.Scale = 0.05f;

		subpass = DeferredRenderer::SubpassBasicMorphGeometry;
	}
	else if constexpr (npc_type == NPC_t::Sphere)
	{
		/* Sphere model is not unit length so scale down collider. */
		Sphere sphere{ 0.5f };
		obj.Collider = Collider{ sphere };

		/* Moment of Intertia of a solid sphere. */
		obj.MoI = Matrix3::CreateScalar((2.0f / 5.0f) * obj.State.Mass * sqr(sphere.Radius));
		obj.State.Cd = 0.5f;
		obj.Scale = 3.0f;

		subpass = DeferredRenderer::SubpassBasicStaticGeometry;
	}
	else if constexpr (npc_type == NPC_t::Box)
	{
		OBB obb{ Vector3{}, Vector3{ 0.5f }, Quaternion{} };
		obj.Collider = Collider{ obb };

		const float w2 = sqr(obb.GetWidth());
		const float h2 = sqr(obb.GetHeight());
		const float d2 = sqr(obb.GetDepth());

		/* Moment of Intertia of a solid box. */
		const float xTensor = recip(12.0f) * obj.State.Mass * (h2 + d2);
		const float yTensor = recip(12.0f) * obj.State.Mass * (w2 + d2);
		const float zTensor = recip(12.0f) * obj.State.Mass * (w2 + h2);
		obj.MoI = Matrix3::CreateScalar(xTensor, yTensor, zTensor);
		obj.State.Cd = 2.1f;
		//obj.Scale = 3.0f;

		subpass = DeferredRenderer::SubpassBasicStaticGeometry;
	}

	npcs.emplace_back(world->AddKinematic(obj, *playerModel, subpass));
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