#include "TestGame.h"
#include <Streams/FileReader.h>
#include <Core/Diagnostics/Stopwatch.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Core/Diagnostics/Profiler.h>
#include <Core/Diagnostics/Memory.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame"), cam(nullptr),
	renderer(nullptr), descPoolConst(nullptr),
	light(nullptr), firstRun(true),
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

void TestGame::LoadContent(void)
{
	renderer = new DeferredRenderer(GetContent(), GetWindow());

	probeRenderer = new LightProbeRenderer(GetContent(), 1);
	environment = new LightProbe(*probeRenderer, Extent2D(256));
	environment->SetPosition(3.88f, 1.37f, 1.11f);

	model = &GetContent().FetchModel(L"{Models}Sponza.pum", *renderer, *probeRenderer);
	skybox = &GetContent().FetchTextureCube(L"Skybox", SamplerCreateInfo{}, true, 
		{
			L"{Textures}Skybox/right.jpg",
			L"{Textures}Skybox/left.jpg",
			L"{Textures}Skybox/top.jpg",
			L"{Textures}Skybox/bottom.jpg",
			L"{Textures}Skybox/front.jpg",
			L"{Textures}Skybox/back.jpg"
		});
}

void TestGame::UnLoadContent(void)
{
	if (cam) delete cam;
	if (light) delete light;
	if (renderer) delete renderer;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;

	GetContent().Release(*model);
	GetContent().Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && probeRenderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		descPoolConst->AddSet(0, 0, 1);	// First camera set
		descPoolConst->AddSet(1, 0, 1); // Second camera set
		descPoolConst->AddSet(2, 0, 1);	// Third camera set
		descPoolConst->AddSet(3, 0, 1); // Forth camera sets
		descPoolConst->AddSet(1, 2, 1);	// Light set

		cam = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		cam->Move(0.0f, 1.0f, -1.0f);
		cam->Yaw = PI2;

		light = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		light->SetEnvironment(environment->GetTexture());

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

	size_t drawCalls = 0;
	if (model->IsLoaded())
	{
		probeRenderer->Initialize(cmd);
		probeRenderer->Start(*environment, cmd);
		size_t i = 0;
		for (const auto &[matIdx, mesh] : model->GetMeshes())
		{
			if (environment->Cull(mesh.GetBoundingBox() * mdlMtrx) || matIdx == -1) continue;

			probeRenderer->Render(mesh, model->GetProbeMaterial(matIdx), mdlMtrx, cmd);
			++drawCalls;
		}
		probeRenderer->End(*environment, cmd);

		renderer->InitializeResources(cmd);
		cam->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		renderer->BeginGeometry(*cam);

		cmd.AddLabel("Model", Color::Blue());
		i = 0;
		renderer->SetModel(mdlMtrx);
		for (const auto[matIdx, mesh] : model->GetMeshes())
		{
			if (cam->GetClip().IntersectionBox(mesh.GetBoundingBox() * mdlMtrx) && matIdx != Model::DefaultMaterialIdx)
			{
				renderer->Render(mesh, model->GetMaterial(matIdx));
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