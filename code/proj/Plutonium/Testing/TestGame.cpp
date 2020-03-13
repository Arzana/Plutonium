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
	descPoolMats(nullptr), vrtxBuffer(nullptr),
	stagingBuffer(nullptr), light(nullptr), firstRun(true),
	updateCam(true), markDepthBuffer(true), mdlMtrx()
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
	Profiler::Begin("Loading", Color::Cyan());
	renderer = new DeferredRenderer(GetContent(), GetWindow());

	const string file = FileReader(L"assets/Models/Sponza.pum").ReadToEnd();
	BinaryReader reader{ file.c_str(), file.length(), Endian::Little };
	PuMData mdl{ GetDevice(), reader };

	animated = false;
	for (const PumAnimation &anim : mdl.Animations)
	{
		if (anim.IsMorphAnimation) animated = true;
	}

	stagingBuffer = mdl.Buffer;
	vrtxBuffer = new Buffer(GetDevice(), stagingBuffer->GetSize(), BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer, false);
	stageMaterials = std::move(mdl.Materials);
	nodes = std::move(mdl.Nodes);

	for (const PumMesh &mesh : mdl.Geometry)
	{
		if (mesh.HasNormals && mesh.HasTangents && mesh.HasTextureCoordinates)
		{
			if (mesh.HasMaterial)
			{
				meshes.emplace_back(std::make_pair(mesh.Material, new Mesh(*vrtxBuffer, mesh)));
			}
			else meshes.emplace_back(std::make_pair(-1, new Mesh(*vrtxBuffer, mesh)));
		}
	}

	std::sort(meshes.begin(), meshes.end(), [](const decltype(meshes)::value_type &a, const decltype(meshes)::value_type &b) { return a.first < b.first; });

	nodeTransforms.resize(nodes.size());
	meshTransforms.resize(meshes.size());

	AssetFetcher &fetcher = GetContent();
	for (const PumTexture &texture : mdl.Textures) textures.emplace_back(&fetcher.FetchTexture2D(texture));
	textures.emplace_back(&fetcher.CreateTexture2D("Default_Diffuse", Color::White()));
	textures.emplace_back(&fetcher.CreateTexture2D("Default_SpecularGlossiness", Color::Black()));
	textures.emplace_back(&fetcher.CreateTexture2D("Default_Normal", Color::Malibu()));
	stageMaterials.emplace_back(PumMaterial());
}

void TestGame::UnLoadContent(void)
{
	AssetFetcher &fetcher = GetContent();

	if (cam) delete cam;
	if (light) delete light;
	if (renderer) delete renderer;

	for (Material *material : materials) delete material;
	if (descPoolConst) delete descPoolConst;
	if (descPoolMats) delete descPoolMats;

	delete vrtxBuffer;
	delete stagingBuffer;

	for (auto[mat, mesh] : meshes) delete mesh;
	for (Texture2D *texture : textures) fetcher.Release(*texture);
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (!renderer->IsUsable()) return;
	for (Texture2D *texture : textures)
	{
		if (!texture->IsUsable()) return;
	}

	if (firstRun)
	{
		firstRun = false;

		descPoolConst = new DescriptorPool(renderer->GetRenderpass());
		descPoolConst->AddSet(0, 0, 1);	// First camera set
		descPoolConst->AddSet(1, 0, 1); // Second camera set
		descPoolConst->AddSet(2, 0, 1); // Third camera sets
		descPoolConst->AddSet(1, 2, 1);	// Light set

		cam = new FreeCamera(GetWindow().GetNative(), *descPoolConst, renderer->GetRenderpass(), GetInput());
		cam->Move(0.0f, 1.0f, -1.0f);
		cam->Yaw = PI2;

		light = new DirectionalLight(*descPoolConst, renderer->GetRenderpass().GetSubpass(1).GetSetLayout(2));

		descPoolMats = new DescriptorPool(renderer->GetRenderpass(), static_cast<uint32>(stageMaterials.size() + 1), 0, 1);
		const DescriptorSetLayout &matLayout = renderer->GetRenderpass().GetSubpass(0).GetSetLayout(1);
		for (const PumMaterial &material : stageMaterials)
		{
			materials.emplace_back(new Material(*descPoolMats, matLayout, material));
		}
		materials.emplace_back(new Material(*descPoolMats, matLayout));

		cmd.CopyEntireBuffer(*stagingBuffer, *vrtxBuffer);
		cmd.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		for (size_t i = 0; i < materials.size() - 1; i++)
		{
			PumMaterial &mat = stageMaterials[i];
			Material &mat2 = *materials[i];

			if (mat.HasDiffuseTexture) mat2.SetDiffuse(*textures[mat.DiffuseTexture]);
			else mat2.SetDiffuse(*textures[textures.size() - 3]);

			if (mat.HasSpecGlossTexture) mat2.SetSpecular(*textures[mat.SpecGlossTexture]);
			else mat2.SetSpecular(*textures[textures.size() - 2]);

			if (mat.HasNormalTexture) mat2.SetNormal(*textures[mat.NormalTexture]);
			else mat2.SetNormal(*textures[textures.size() - 1]);

		}

		Material &defMat = *materials.back();
		defMat.SetDiffuse(*textures[textures.size() - 3]);
		defMat.SetSpecular(*textures[textures.size() - 2]);
		defMat.SetNormal(*textures[textures.size() - 1]);

		descPoolMats->Update(cmd, PipelineStageFlag::FragmentShader);
		Profiler::End();

		if (!nodeTransforms.empty())
		{
			SetNodeTransform(0, mdlMtrx);
			SetMeshTransforms();
		}
	}

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

	renderer->InitializeResources(cmd);
	cam->Update(dt * updateCam);
	descPoolConst->Update(cmd, PipelineStageFlag::VertexShader);

	renderer->BeginGeometry(*cam);

	cmd.AddLabel("Model", Color::Blue());
	size_t i = 0, drawCalls = 0;
	for (const auto[matIdx, mesh] : meshes)
	{
		const Matrix transform = meshTransforms[i++];
		if (cam->GetClip().IntersectionBox(mesh->GetBoundingBox() * transform))
		{
			renderer->SetModel(transform);
			renderer->Render(*mesh, *(matIdx != -1 ? materials[matIdx] : materials.back()));
			++drawCalls;
			if (animated) break;
		}
	}

	cmd.EndLabel();
	renderer->BeginLight();
	renderer->Render(*light);
	renderer->End();

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

void TestGame::SetNodeTransform(size_t idx, const Matrix & parent)
{
	const PumNode &node = nodes[idx];
	const Matrix transform = parent * node.GetTransform();
	nodeTransforms[idx] = transform;

	for (size_t child : node.Children)
	{
		SetNodeTransform(child, transform);
	}
}

void TestGame::SetMeshTransforms(void)
{
	for (size_t i = 0; i < meshes.size(); i++)
	{
		meshTransforms[i] = nodeTransforms.front();

		for (size_t j = 0; j < nodes.size(); j++)
		{
			const PumNode &node = nodes[j];
			if (node.HasMesh && node.Mesh == i)
			{
				meshTransforms[i] = nodeTransforms[j];
				break;
			}
		}
	}
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