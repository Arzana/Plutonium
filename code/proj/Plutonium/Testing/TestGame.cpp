#include "TestGame.h"
#include <Graphics/VertexLayouts/Advanced3D.h>
#include <Graphics/VertexLayouts/Basic3D.h>
#include <Core/Diagnostics/Profiler.h>
#include <Core/Diagnostics/Memory.h>
#include <imgui.h>

#include <Streams/FileReader.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame (Unlit!)"), cam(nullptr),
	renderer(nullptr), descPoolConst(nullptr),
	light(nullptr), firstRun(true), updateCam(true),
	markDepthBuffer(true), mdlMtrx(Matrix::CreateScalar(0.008f))
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
}

void TestGame::EnableFeatures(const Pu::PhysicalDeviceFeatures & supported, Pu::PhysicalDeviceFeatures & enabeled)
{
	if (supported.WideLines) enabeled.WideLines = true;					// Debug renderer
	if (supported.FillModeNonSolid) enabeled.FillModeNonSolid = true;	// Easy wireframe mode
	if (supported.MultiDrawIndirect) enabeled.MultiDrawIndirect = true;	// Allows for less draw calls.

	enabeled.SamplerAnisotropy = true;	// Textures are loaded with 4 anisotropy by default
	enabeled.GeometryShader = true;		// Needed for the light probe renderer.
}

void TestGame::Initialize(void)
{
#ifdef _DEBUG
	GetWindow().SetMode(WindowMode::Borderless);
#endif

	Mouse::HideAndLockCursor(GetWindow().GetNative());
}

void TestGame::LoadContent(AssetFetcher & fetcher)
{
	renderer = new DeferredRenderer(fetcher, GetWindow());
	dbgRenderer = new DebugRenderer(GetWindow(), fetcher, &renderer->GetDepthBuffer(), 2.0f);

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
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (cam) delete cam;
	if (light) delete light;
	if (renderer) delete renderer;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;
	if (dbgRenderer) delete dbgRenderer;

	fetcher.Release(*model);
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && skybox->IsUsable())
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
		light->SetEnvironment(*skybox);

		renderer->SetSkybox(*skybox);
	}
	else if (!firstRun)
	{
		if (ImGui::Begin("Light Editor"))
		{
			float intensity = light->GetIntensity();
			if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) light->SetIntensity(intensity);

			Vector3 clr = light->GetRadiance();
			if (ImGui::ColorPicker3("Radiance", clr.f)) light->SetRadiance(clr);

			float contrast = cam->GetContrast();
			if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 10.0f)) cam->SetContrast(contrast);

			float brightness = cam->Brightness();
			if (ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f)) cam->SetBrightness(brightness);

			float exposure = cam->GetExposure();
			if (ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f)) cam->SetExposure(exposure);

			ImGui::End();
		}

		dbgRenderer->AddTransform(light->GetView(), 0.25f, Vector3::Up());
	}

	if (model->IsLoaded() && cam)
	{
		renderer->InitializeResources(cmd);
		cam->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		renderer->BeginGeometry(*cam);
		renderer->Render(*model, mdlMtrx);

		renderer->BeginAdvanced();
		renderer->Render(*model, mdlMtrx);

		renderer->BeginLight();
		renderer->Render(*light);
		renderer->End();

		dbgRenderer->Render(cmd, cam->GetProjection(), cam->GetView());
	}

	if (ImGui::BeginMainMenuBar())
	{
		const MemoryFrame cpuStats = MemoryFrame::GetCPUMemStats();
		const MemoryFrame gpuStats = MemoryFrame::GetGPUMemStats(GetDevice().GetPhysicalDevice());

		ImGui::Text("FPS: %d", iround(recip(dt)));
		ImGui::Separator();
		ImGui::Text("Bind/Draw Calls: %u/%u", renderer->GetBindCount(), renderer->GetDrawCount());
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