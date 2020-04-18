#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <Core/Diagnostics/Memory.h>
#include <imgui.h>

#include <Graphics/Models/ShapeCreator.h>
#include <Core/Math/PerlinNoise.h>

using namespace Pu;

constexpr float pathScale = 8.0f;
constexpr uint16 patchSize = 16;
constexpr uint16 imgSize = patchSize << 6;

TestGame::TestGame(void)
	: Application(L"TestGame (Bad PBR)"), cam(nullptr),
	renderer(nullptr), descPoolConst(nullptr),
	lightMain(nullptr), lightFill(nullptr), firstRun(true), updateCam(true),
	markDepthBuffer(true), heightMap(imgSize, recip(pathScale))
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

	srcBuffer = new StagingBuffer(GetDevice(), ShapeCreator::GetPatchPlaneBufferSize(patchSize));
	terrainMesh.Initialize(GetDevice(), *srcBuffer, ShapeCreator::GetPatchPlaneVertexSize(patchSize), ShapeCreator::PatchPlane(*srcBuffer, patchSize));

	mask = &fetcher.CreateTexture2D("Mask", Color{ (byte)0, 255, 0, 0 });
	textures = &fetcher.FetchTexture2DArray("Terrain", SamplerCreateInfo{}, true,
		{
			L"{Textures}Terrain/Water.jpg",
			L"{Textures}Terrain/Grass.jpg",
			L"{Textures}Terrain/Dirt.jpg",
			L"{Textures}Terrain/Snow.jpg"
		});

	heightSampler = &fetcher.FetchSampler(SamplerCreateInfo{ Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToEdge });
	heightImg = new Image(GetDevice(), ImageCreateInfo{ ImageType::Image2D, Format::R32_SFLOAT, Extent3D{imgSize, 1}, 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled });
	height = new Texture2D(*heightImg, *heightSampler);

	heightBuffer = new StagingBuffer(GetDevice(), sqr(imgSize) * sizeof(float));
	heightBuffer->BeginMemoryTransfer();
	float *pixels = reinterpret_cast<float*>(heightBuffer->GetHostMemory());

	float mi = maxv<float>(), ma = minv<float>();
	PerlinNoise noise{};
	for (size_t y = 0, i = 0; y < imgSize; y++)
	{
		for (size_t x = 0; x < imgSize; x++, i++)
		{
			constexpr float isize = recip(imgSize);
			pixels[i] = noise.NormalizedScale(x * isize, y * isize, 4, 0.5f, 2.0f);
			heightMap.SetHeight(i, pixels[i]);

			mi = min(mi, pixels[i]);
			ma = max(ma, pixels[i]);
		}
	}

	for (size_t i = 0; i < sqr(imgSize); i++) pixels[i] = ilerp(mi, ma, pixels[i]);

	heightBuffer->EndMemoryTransfer();
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
	if (srcBuffer) delete srcBuffer;
	if (terrainMat) delete terrainMat;
	if (height) delete height;
	if (heightImg) delete heightImg;
	if (heightBuffer) delete heightBuffer;

	fetcher.Release(*heightSampler);
	fetcher.Release(*mask);
	fetcher.Release(*textures);
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && probeRenderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;
		cmd.CopyEntireBuffer(*srcBuffer, terrainMesh.GetBuffer());

		cmd.MemoryBarrier(*heightImg, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, height->GetFullRange());
		cmd.CopyEntireBuffer(*heightBuffer, *heightImg);
		cmd.MemoryBarrier(*heightImg, PipelineStageFlag::Transfer, PipelineStageFlag::TessellationControlShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, height->GetFullRange());

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		renderer->InitializeCameraPool(*descPoolConst, 1);						// Camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);	// Light set
		descPoolConst->AddSet(DeferredRenderer::SubpassTerrain, 1, 1);			// Terrain set

		cam = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		cam->Move(0.0f, 1.0f, -1.0f);
		cam->Yaw = PI2;
		cam->SetExposure(2.5f);

		lightMain = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightMain->SetEnvironment(environment->GetTexture());

		lightFill = new DirectionalLight(*descPoolConst, renderer->GetDirectionalLightLayout());
		lightFill->SetDirection(normalize(Vector3(0.7f)));
		lightFill->SetIntensity(0.5f);
		lightFill->SetEnvironment(environment->GetTexture());

		terrainMat = new Terrain(*descPoolConst, renderer->GetTerrainLayout());
		terrainMat->SetHeight(*height);
		terrainMat->SetMask(*mask);
		terrainMat->SetTextures(*textures);
		terrainMat->SetScale(pathScale);

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

		if (ImGui::Begin("Terrain Editor"))
		{
			float displ = terrainMat->GetDisplacement();
			if (ImGui::SliderFloat("Displacement", &displ, 0.0f, 50.0f)) terrainMat->SetDisplacement(displ);

			float tess = terrainMat->GetTessellation();
			if (ImGui::SliderFloat("Tessellation", &tess, 0.0f, 2.0f)) terrainMat->SetTessellation(tess);

			float edge = terrainMat->GetEdgeSize();
			if (ImGui::SliderFloat("Edge Size", &edge, 0.0f, 40.0f)) terrainMat->SetEdgeSize(edge);

			ImGui::End();

			AABB terrain = terrainMesh.GetBoundingBox();
			terrain.UpperBound.Y += displ;
			dbgRenderer->AddBox(terrain, terrainMat->GetTransform(), Color::Yellow());

			const Vector2 pos = Vector2{ cam->GetPosition().X, cam->GetPosition().Z };
			const Vector2 relPos = pos + Vector2{ patchSize / 2 * pathScale };
			float h;
			if (heightMap.TryGetHeight(relPos, h))
			{
				dbgRenderer->AddSphere(Vector3{ pos.X, h * displ * pathScale, pos.Y }, 1.0f, Color::Red());
			}
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
		renderer->Render(terrainMesh, *terrainMat);
		renderer->BeginGeometry();
		renderer->BeginAdvanced();
		renderer->BeginLight();
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();
	}

	dbgRenderer->Render(cmd, cam->GetProjection(), cam->GetView());

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

void TestGame::OnSwapchainRecreated(const Pu::GameWindow &, const Pu::SwapchainReCreatedEventArgs &)
{
	if (dbgRenderer) dbgRenderer->Reset(renderer->GetDepthBuffer());
}