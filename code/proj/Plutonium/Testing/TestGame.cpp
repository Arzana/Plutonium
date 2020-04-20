#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <Core/Diagnostics/Memory.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame (Bad PBR)"), cam(nullptr),
	renderer(nullptr), descPoolConst(nullptr), updateCam(true),
	lightMain(nullptr), lightFill(nullptr), firstRun(true)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
}

void TestGame::EnableFeatures(const Pu::PhysicalDeviceFeatures & supported, Pu::PhysicalDeviceFeatures & enabeled)
{
	if (supported.WideLines) enabeled.WideLines = true;					// Debug renderer
	if (supported.FillModeNonSolid) enabeled.FillModeNonSolid = true;	// Easy wireframe mode

	enabeled.SamplerAnisotropy = true;	// Textures are loaded with 4 anisotropy by default
	enabeled.GeometryShader = true;		// Needed for the light probe renderer.
	enabeled.TessellationShader = true;	// Needed for terrain rendering.
}

void TestGame::Initialize(void)
{
	Mouse::HideAndLockCursor(GetWindow().GetNative());
}

void TestGame::LoadContent(AssetFetcher & fetcher)
{
	renderer = new DeferredRenderer(fetcher, GetWindow());
	dbgRenderer = new DebugRenderer(GetWindow(), fetcher, &renderer->GetDepthBuffer(), 2.0f);
	GetWindow().SwapchainRecreated.Add(*this, &TestGame::OnSwapchainRecreated);

	probeRenderer = new LightProbeRenderer(fetcher, 1);
	environment = new LightProbe(*probeRenderer, Extent2D(256));
	environment->SetPosition(0.0f, 1.0f, 0.0f);

	skybox = &fetcher.FetchSkybox(
		{
			L"{Textures}Skybox/right.jpg",
			L"{Textures}Skybox/left.jpg",
			L"{Textures}Skybox/top.jpg",
			L"{Textures}Skybox/bottom.jpg",
			L"{Textures}Skybox/front.jpg",
			L"{Textures}Skybox/back.jpg"
		});

	ground = &fetcher.CreateModel(ShapeType::Plane, *renderer, *probeRenderer);
	ball = &fetcher.CreateModel(ShapeType::Sphere, *renderer, *probeRenderer, L"{Textures}uv.png");
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (cam) delete cam;
	if (lightMain) delete lightMain;
	if (lightFill) delete lightFill;
	if (renderer) delete renderer;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;
	if (dbgRenderer) delete dbgRenderer;

	fetcher.Release(*ball);
	fetcher.Release(*ground);
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && probeRenderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		renderer->InitializeCameraPool(*descPoolConst, 1);						// Camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);	// Light set

		cam = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		cam->Move(0.0f, 1.0f, -1.0f);
		cam->SetExposure(2.5f);

		lightMain = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightMain->SetEnvironment(environment->GetTexture());

		lightFill = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightFill->SetDirection(normalize(Vector3(0.7f)));
		lightFill->SetIntensity(0.5f);
		lightFill->SetEnvironment(environment->GetTexture());

		renderer->SetSkybox(*skybox);
	}
	else if (!firstRun)
	{
		if (ImGui::Begin("Light Editor"))
		{
			float intensity = lightMain->GetIntensity();
			if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) lightMain->SetIntensity(intensity);

			Vector3 clr = lightMain->GetRadiance();
			if (ImGui::ColorPicker3("Radiance", clr.f)) lightMain->SetRadiance(clr);

			float contrast = cam->GetContrast();
			if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 10.0f)) cam->SetContrast(contrast);

			float brightness = cam->Brightness();
			if (ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f)) cam->SetBrightness(brightness);

			float exposure = cam->GetExposure();
			if (ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f)) cam->SetExposure(exposure);

			ImGui::End();
		}
	}

	if (cam)
	{
		cam->Update(dt * updateCam);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		if (environment->ShouldUpdate(dt))
		{
			probeRenderer->Initialize(cmd);
			probeRenderer->Start(*environment, cmd);
			probeRenderer->End(*environment, cmd);
		}

		renderer->InitializeResources(cmd);
		renderer->BeginTerrain(*cam);
		renderer->BeginGeometry();
		if (ground->IsLoaded()) renderer->Render(*ground, Matrix::CreatePitch(PI2) * Matrix::CreateScalar(10.0f));
		if (ball->IsLoaded()) renderer->Render(*ball, Matrix::CreateTranslation(0.0f, 5.0f, 0.0f));
		renderer->BeginAdvanced();
		renderer->BeginLight();
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();

		dbgRenderer->Render(cmd, cam->GetProjection(), cam->GetView());
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

void TestGame::OnSwapchainRecreated(const Pu::GameWindow &, const Pu::SwapchainReCreatedEventArgs &)
{
	if (dbgRenderer) dbgRenderer->Reset(renderer->GetDepthBuffer());
}