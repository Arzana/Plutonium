#include "TestGame.h"
#include <Core/Diagnostics/Profiler.h>
#include <imgui.h>

#include <Graphics/Models/ShapeCreator.h>
#include <Core/Math/PerlinNoise.h>

using namespace Pu;

const uint16 meshSize = 64;
const float meshScale = 10.0f;
const float imgScale = 16.0f;
const uint16 imgSize = meshSize * imgScale;
const float displacement = 5.0f;

TestGame::TestGame(void)
	: Application(L"TestGame (Bad PBR)"),
	updateCam(false), firstRun(true), playerWorld(Matrix::CreateTranslation(0.0f, 100.0f, 0.0f)),
	mass(1.0f), e(0.5f), time(1.0f), speed(20.0f)
{
	// Inertia of a solid sphere = 2/5 * mr^2
	imass = recip(mass);
	MoI = Matrix::CreateScalar((2.0f / 5.0f) * recip(imass) * sqr(0.5f)).GetInverse();

	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	GetInput().AnyKeyUp.Add(*this, &TestGame::OnAnyKeyUp);
	GetInput().AnyMouseScrolled.Add(*this, &TestGame::OnMouseScrolled);
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

	playerModel = &fetcher.FetchModel(L"{Models}knight.pum", *renderer, *probeRenderer);

	/* Additional terrain textures. */
	mask = &fetcher.CreateTexture2D("TerrainMask", Color::Black());
	groundTextures = &fetcher.FetchTexture2DArray("TerrainTextures", SamplerCreateInfo{}, true,
		{
			L"{Textures}Terrain/Water.jpg",
			L"{Textures}Terrain/Grass.jpg",
			L"{Textures}Terrain/Dirt.jpg",
			L"{Textures}Terrain/Snow.jpg"
		});

	/* Terrain mesh. */
	groundMeshStagingBuffer = new StagingBuffer(GetDevice(), ShapeCreator::GetPatchPlaneBufferSize(meshSize));
	Mesh mesh = ShapeCreator::PatchPlane(*groundMeshStagingBuffer, meshSize);
	groundMesh.Initialize(GetDevice(), *groundMeshStagingBuffer, ShapeCreator::GetPatchPlaneVertexSize(meshSize), std::move(mesh));

	/* Perlin height map. */
	perlinStagingBuffer = new StagingBuffer(GetDevice(), imgSize * imgSize * sizeof(float));
	perlinImg = new Image(GetDevice(), ImageCreateInfo{ ImageType::Image2D, Format::R32_SFLOAT, Extent3D(imgSize, imgSize, 1), 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::Sampled | ImageUsageFlag::TransferDst });
	perlinSampler = &fetcher.FetchSampler(SamplerCreateInfo{ Filter::Nearest, SamplerMipmapMode::Nearest, SamplerAddressMode::ClampToEdge });
	perlin = new Texture2D(*perlinImg, *perlinSampler);

	collider = new HeightMap(imgSize, imgScale);
	perlinStagingBuffer->BeginMemoryTransfer();
	float *pixel = reinterpret_cast<float*>(perlinStagingBuffer->GetHostMemory());

	PerlinNoise noise;
	const float step = recip(static_cast<float>(imgSize));
	size_t i = 0;
	float mi = maxv<float>(), ma = minv<float>();
	for (float y = 0; y < 1.0f; y += step)
	{
		for (float x = 0; x < 1.0f; x += step)
		{
			const float h = noise.NormalizedScale(x, y, 4, 0.5f, 2.0f);

			pixel[i++] = h;
			mi = min(mi, h);
			ma = max(ma, h);
		}
	}

	i = 0;
	for (uint16 y = 0; y < imgSize; y++)
	{
		for (uint16 x = 0; x < imgSize; x++)
		{
			pixel[i] = ilerp(mi, ma, pixel[i]);
			collider->SetHeight(x, y, pixel[i++]);
		}
	}

	perlinStagingBuffer->EndMemoryTransfer();
	collider->CalculateNormals(displacement);
}

void TestGame::UnLoadContent(AssetFetcher & fetcher)
{
	if (camFree) delete camFree;
	if (camFollow) delete camFollow;
	if (lightMain) delete lightMain;
	if (lightFill) delete lightFill;
	if (renderer) delete renderer;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;
	if (dbgRenderer) delete dbgRenderer;

	if (collider) delete collider;
	if (perlinImg) delete perlinImg;
	fetcher.Release(*perlinSampler);
	if (perlin) delete perlin;
	fetcher.Release(*mask);
	fetcher.Release(*groundTextures);
	if (groundMeshStagingBuffer) delete groundMeshStagingBuffer;
	if (perlinStagingBuffer) delete perlinStagingBuffer;

	fetcher.Release(*playerModel);
	fetcher.Release(*skybox);
	if (descPoolConst) delete descPoolConst;
}

void TestGame::Update(float dt)
{
	Profiler::Begin("Physics", Color::Gray());
	/* Gravity. */
	dt *= time;
	vloc.Y -= 9.8f * dt;

	/* Player input. */
	Vector3 pos = playerWorld.GetTranslation();
	pos += input * dt * speed;

	/* Terrain collision. */
	float h;
	Vector2 relPos = Vector2(pos.X, pos.Z) + Vector2(meshSize * 0.5f * meshScale);
	if (collider->TryGetHeight(Vector2(relPos / meshScale), h))
	{
		/* Simple positional constraint. */
		const float relH = h * displacement * meshScale + 1.5f;
		if (pos.Y < relH)
		{
			pos.Y = relH;
			vloc.Y = 0.0f;
		}
	}

	///* Linear impulse. */
	//const float jl = max(0.0f, -(1.0f + e) * dot(vloc, Vector3::Up()));
	//vloc += jl * Vector3::Up();

	//	/* Angular impulse. */
	//	const Vector3 r = Vector3() - pos;
	//	float nom = -(1.0f + e) * dot(vloc, collider.N);
	//	const float denom = imass + dot(iI * (r * collider.N) * r, collider.N);
	//	const float jr = nom / denom;
	//	angularVloc += iI * -cross(r, collider.N * jr);
	//}

	pos += vloc * dt;
	//rot += Quaternion::Create(angularVloc.Y, angularVloc.X, angularVloc.Z) * dt;
	//rot.Normalize();

	playerWorld = Matrix::CreateWorld(pos, Quaternion{ SQRT05, 0.0f, -SQRT05, 0.0f }, Vector3(0.05f));
	Profiler::End();
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (firstRun && renderer->IsUsable() && probeRenderer->IsUsable() && skybox->IsUsable())
	{
		firstRun = false;

		cmd.MemoryBarrier(groundMesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, AccessFlag::TransferWrite);
		cmd.CopyEntireBuffer(*groundMeshStagingBuffer, groundMesh.GetBuffer());
		cmd.MemoryBarrier(groundMesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		cmd.MemoryBarrier(*perlinImg, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, perlin->GetFullRange());
		cmd.CopyEntireBuffer(*perlinStagingBuffer, *perlinImg);
		cmd.MemoryBarrier(*perlinImg, PipelineStageFlag::Transfer, PipelineStageFlag::TessellationControlShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, perlin->GetFullRange());

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		renderer->InitializeCameraPool(*descPoolConst, 2);						// Camera sets
		descPoolConst->AddSet(DeferredRenderer::SubpassDirectionalLight, 2, 2);	// Light set
		descPoolConst->AddSet(DeferredRenderer::SubpassTerrain, 1, 1);			// Terrain set

		groundMat = new Terrain(*descPoolConst, renderer->GetTerrainLayout());
		groundMat->SetHeight(*perlin);
		groundMat->SetMask(*mask);
		groundMat->SetTextures(*groundTextures);
		groundMat->SetDisplacement(displacement);
		groundMat->SetScale(meshScale);

		camFree = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		camFree->Move(5.0f, 1.0f, -5.0f);
		camFree->SetExposure(2.5f);
		camFree->Yaw = TAU - PI4;

		camFollow = new FollowCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass());
		camFollow->SetTarget(playerWorld);
		camActive = camFollow;

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
		camFree->Update(dt * updateCam);
		camFollow->Update(dt);
		descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

		if (environment->ShouldUpdate(dt))
		{
			probeRenderer->Initialize(cmd);
			probeRenderer->Start(*environment, cmd);
			probeRenderer->End(*environment, cmd);
		}

		renderer->InitializeResources(cmd);
		renderer->BeginTerrain(*camActive);
		if (mask->IsUsable() && groundTextures->IsUsable()) renderer->Render(groundMesh, *groundMat);
		renderer->BeginGeometry();
		if (playerModel->IsLoaded()) renderer->Render(*playerModel, playerWorld);
		renderer->BeginAdvanced();
		renderer->BeginLight();
		renderer->Render(*lightMain);
		renderer->Render(*lightFill);
		renderer->End();

		dbgRenderer->AddBox(playerModel->GetMeshes().GetBoundingBox(0), playerWorld, Color::Yellow());
		dbgRenderer->Render(cmd, *camActive);
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
		else if (args.Key == Keys::NumAdd) camFree->MoveSpeed++;
		else if (args.Key == Keys::NumSubtract) camFree->MoveSpeed--;
		else if (args.Key == Keys::D1 && camActive != camFree)
		{
			updateCam = true;
			camActive = camFree;
		}
		else if (args.Key == Keys::D2) camActive = camFollow;

		if (args.Key == Keys::W) input.Z = 1.0f;
		if (args.Key == Keys::S) input.Z = -1.0f;
		if (args.Key == Keys::A) input.X = -1.0f;
		if (args.Key == Keys::D) input.X = 1.0f;
	}
	else if (sender.Type == InputDeviceType::GamePad)
	{
		if (args.Key == Keys::XBoxB) Exit();
	}
}

void TestGame::OnAnyKeyUp(const Pu::InputDevice & sender, const Pu::ButtonEventArgs & args)
{
	if (sender.Type == InputDeviceType::Keyboard)
	{
		if (args.Key == Keys::W) input.Z = 0.0f;
		if (args.Key == Keys::S) input.Z = 0.0f;
		if (args.Key == Keys::A) input.X = 0.0f;
		if (args.Key == Keys::D) input.X = 0.0f;
	}
}

void TestGame::OnMouseScrolled(const Mouse&, int16 args)
{
	if (camFollow)
	{
		camFollow->Distance -= args;
		camFollow->Height -= args;
	}
}

void TestGame::OnSwapchainRecreated(const GameWindow &, const SwapchainReCreatedEventArgs &)
{
	if (dbgRenderer) dbgRenderer->Reset(renderer->GetDepthBuffer());
}