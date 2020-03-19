#include "TestGame.h"
#include <Streams/FileReader.h>
#include <Core/Diagnostics/Stopwatch.h>
#include <Graphics/VertexLayouts/Advanced3D.h>
#include <Graphics/VertexLayouts/Basic3D.h>
#include <Core/Diagnostics/Profiler.h>
#include <Core/Diagnostics/Memory.h>
#include <Core/Math/PerlinNoise.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame"), cam(nullptr),
	renderer(nullptr), descPoolConst(nullptr),
	light(nullptr), firstRun(true), noiseTexture(nullptr),
	updateCam(true), markDepthBuffer(true), mdlMtrx(Matrix::CreateScalar(0.008f))
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
}

void TestGame::EnableFeatures(PhysicalDeviceFeatures & features)
{
	features.LogicOp = true;			// Needed for blending
	features.WideLines = true;			// Debug renderer
	features.FillModeNonSolid = true;	// Easy wireframe mode
	features.SamplerAnisotropy = true;	// Textures are loaded with 4 anisotropy by default
	features.GeometryShader = true;		// Needed for the light probe renderer.
}

void TestGame::Initialize(void)
{
	GetWindow().SetMode(WindowMode::Borderless);
	Mouse::HideAndLockCursor(GetWindow().GetNative());
}

void TestGame::LoadContent(AssetFetcher & fetcher)
{
	renderer = new DeferredRenderer(fetcher, GetWindow());

	probeRenderer = new LightProbeRenderer(fetcher, 1);
	environment = new LightProbe(*probeRenderer, Extent2D(256));

	model = &fetcher.FetchModel(L"{Models}Sponza.pum", *renderer, *probeRenderer);
	skybox = &fetcher.FetchSkybox(
		{
			L"{Textures}Skybox/right.jpg",
			L"{Textures}Skybox/left.jpg",
			L"{Textures}Skybox/top.jpg",
			L"{Textures}Skybox/bottom.jpg",
			L"{Textures}Skybox/front.jpg",
			L"{Textures}Skybox/back.jpg"
		});

	const uint32 w = 256, h = 256;
	uint8 *buffer = reinterpret_cast<uint8*>(malloc(w * h * sizeof(uint8)));
	PerlinNoise noise;

	for (uint32 y = 0; y < h; y++)
	{
		for (uint32 x = 0; x < w; x++)
		{
			const float py = y / static_cast<float>(h);
			const float px = x / static_cast<float>(w);
			buffer[y * w + x] = static_cast<uint8>(noise.NormalizedScale(px, py, 4, 0.5f, 2.0f) * maxv<uint8>());
		}
	}

	noiseTexture = &fetcher.CreateTexture2D("Noise", buffer, w, h, Format::R8_UINT, SamplerCreateInfo{});
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (cam) delete cam;
	if (light) delete light;
	if (renderer) delete renderer;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;

	fetcher.Release(*model);
	fetcher.Release(*skybox);
	fetcher.Release(*noiseTexture);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	size_t drawCalls = 0;

	if (firstRun && renderer->IsUsable() && probeRenderer->IsUsable() && skybox->IsUsable() && model->IsLoaded())
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		descPoolConst->AddSet(DeferredRenderer::SubpassAdvancedStaticGeometry, 0, 1);	// First camera set
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 0, 1);			// Second camera set
		descPoolConst->AddSet(DeferredRenderer::SubpassSkybox, 0, 1);					// Third camera set
		descPoolConst->AddSet(DeferredRenderer::SubpassPostProcessing, 0, 1);			// Forth camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 1);			// Light set

		cam = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		cam->Move(0.0f, 1.0f, -1.0f);
		cam->Yaw = PI2;

		light = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		light->SetEnvironment(environment->GetTexture());

		renderer->SetSkybox(*skybox);

		probeRenderer->Initialize(cmd);
		probeRenderer->Start(*environment, cmd);
		for (const auto &[matIdx, mesh] : model->GetAdvancedMeshes())
		{
			if (environment->Cull(mesh.GetBoundingBox() * mdlMtrx) || matIdx == -1) continue;

			probeRenderer->Render(mesh, model->GetProbeMaterial(matIdx), mdlMtrx, cmd);
			++drawCalls;
		}
		probeRenderer->End(*environment, cmd);
	}
	else if (!firstRun)
	{
		if (ImGui::Begin("Light Editor"))
		{
			float intensity = light->GetIntensity();
			if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) light->SetIntensity(intensity);

			Vector3 clr = light->GetRadiance();
			if (ImGui::ColorPicker3("Radiance", clr.f)) light->SetRadiance(clr);

			Vector3 dir = light->GetDirection();
			if (ImGui::SliderFloat3("Direction", dir.f, 0.0f, 1.0f)) light->SetDirection(dir.X, dir.Y, dir.Z);

			float contrast = cam->GetContrast();
			if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 10.0f)) cam->SetContrast(contrast);

			float brightness = cam->Brightness();
			if (ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f)) cam->SetBrightness(brightness);

			float exposure = cam->GetExposure();
			if (ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f)) cam->SetExposure(exposure);

			ImGui::End();
		}
	}

	if (model->IsLoaded() && cam)
	{
		renderer->InitializeResources(cmd);
		cam->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		renderer->BeginGeometry(*cam);

		cmd.AddLabel("Model", Color::Blue());
		renderer->SetModel(mdlMtrx);
		for (const auto[mat, mesh] : model->GetBasicMeshes())
		{
			if (cam->GetClip().IntersectionBox(mesh.GetBoundingBox() * mdlMtrx) && mat != Model::DefaultMaterialIdx)
			{
				renderer->Render(mesh, model->GetMaterial(mat));
				++drawCalls;
			}
		}

		renderer->BeginAdvanced();
		renderer->SetModel(mdlMtrx);
		for (const auto[mat, mesh] : model->GetAdvancedMeshes())
		{
			if (cam->GetClip().IntersectionBox(mesh.GetBoundingBox() * mdlMtrx) && mat != Model::DefaultMaterialIdx)
			{
				renderer->Render(mesh, model->GetMaterial(mat));
				++drawCalls;
			}
		}

		cmd.EndLabel();
		renderer->BeginLight();
		renderer->Render(*light);
		renderer->End();
	}

	if (ImGui::BeginMainMenuBar())
	{
		const MemoryFrame cpuStats = MemoryFrame::GetCPUMemStats();
		const MemoryFrame gpuStats = MemoryFrame::GetGPUMemStats(GetDevice().GetPhysicalDevice());

		ImGui::Text("FPS: %d", iround(recip(dt)));
		ImGui::Separator();
		ImGui::Text("Draw Calls: %zu", drawCalls);
		ImGui::Separator();
		ImGui::Text("CPU: %zu MB / %zu MB", b2mb(cpuStats.UsedVRam), b2mb(cpuStats.TotalVRam));
		ImGui::Separator();
		ImGui::Text("GPU %zu MB / %zu MB", b2mb(gpuStats.UsedVRam), b2mb(gpuStats.TotalVRam));

		ImGui::EndMainMenuBar();
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
			if (updateCam)
			{
				Mouse::ShowAndFreeCursor();
				updateCam = false;
			}
			else
			{
				Mouse::HideAndLockCursor(GetWindow().GetNative());
				updateCam = true;
			}
		}
		else if (args.Key == Keys::NumAdd) cam->MoveSpeed++;
		else if (args.Key == Keys::NumSubtract) cam->MoveSpeed--;
	}
	else if (sender.Type == InputDeviceType::GamePad)
	{
		if (args.Key == Keys::XBoxB) Exit();
	}
}