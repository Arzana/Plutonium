#include "TestGame.h"
#include <Input/Keys.h>
#include <Streams/FileReader.h>
#include <Core/Diagnostics/Stopwatch.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Core/Diagnostics/Profiler.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame"), cam(nullptr),
	renderPass(nullptr), gfxPipeline(nullptr), depthBuffer(nullptr),
	descPoolCam(nullptr), descPoolMats(nullptr), vrtxBuffer(nullptr),
	stagingBuffer(nullptr), light(nullptr), firstRun(true), updateCam(true),
	markDepthBuffer(true), mdlMtrx(/*Matrix::CreateScalar(0.05f)*/), dbgRenderer(nullptr)
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
	GetWindow().SwapchainRecreated.Add(*this, &TestGame::OnSwapchainRecreated);
	GetWindow().SetMode(WindowMode::Borderless);
	Mouse::HideAndLockCursor(GetWindow().GetNative());

	queries = new QueryPool(GetDevice(), QueryType::Timestamp, 4);
}

void TestGame::LoadContent(void)
{
	Profiler::Begin("Loading", Color::Cyan());
	const string file = FileReader(L"assets/Models/Sponza.pum").ReadToEnd();
	BinaryReader reader{ file.c_str(), file.length(), Endian::Little };
	PuMData mdl{ GetDevice(), reader };

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

	AssetFetcher &fetcher = GetContent();
	for (const PumTexture &texture : mdl.Textures) textures.emplace_back(&fetcher.FetchTexture2D(texture));

	probeRenderer = new LightProbeRenderer(fetcher, 1);
	environment = new LightProbe(*probeRenderer, Extent2D(256, 256));
	environment->SetPosition(Vector3(3.88f, 1.37f, 1.11f));

	textures.emplace_back(&fetcher.CreateTexture2D("Default_Diffuse_Occlusion", Color::White()));
	textures.emplace_back(&fetcher.CreateTexture2D("Default_SpecularGlossiness_Emisive", Color::Black()));
	textures.emplace_back(&fetcher.CreateTexture2D("Default_Normal", Color::Malibu()));
	stageMaterials.emplace_back(PumMaterial());

	renderPass = &GetContent().FetchRenderpass({ { L"{Shaders}Forward3D.vert.spv", L"{Shaders}Forward3D.frag.spv" } });
	renderPass->PreCreate.Add(*this, &TestGame::InitializeRenderpass);
	renderPass->PostCreate.Add(*this, &TestGame::FinalizeRenderpass);
}

void TestGame::UnLoadContent(void)
{
	AssetFetcher &fetcher = GetContent();

	for (DescriptorSet &cur : probeSets) probePool->DeAllocate(cur);
	delete probePool;

	if (cam) delete cam;
	if (light) delete light;
	if (environment) delete environment;
	if (probeRenderer) delete probeRenderer;

	for (Material *material : materials) delete material;
	if (descPoolCam) delete descPoolCam;
	if (descPoolMats) delete descPoolMats;
	if (descPoolLight) delete descPoolLight;
	if (gfxPipeline) delete gfxPipeline;

	delete vrtxBuffer;
	delete stagingBuffer;
	fetcher.Release(*renderPass);

	for (auto[mat, mesh] : meshes) delete mesh;
	for (Texture2D *texture : textures) fetcher.Release(*texture);
}

void TestGame::Finalize(void)
{
	delete queries;

	if (depthBuffer)
	{
		delete depthBuffer;
		delete dbgRenderer;
	}
}

void TestGame::Render(float dt, CommandBuffer &cmd)
{
	if (!gfxPipeline) return;
	if (!gfxPipeline->IsUsable()) return;

	for (Texture2D *texture : textures)
	{
		if (!texture->IsUsable()) return;
	}

	if (markDepthBuffer)
	{
		markDepthBuffer = false;
		depthBuffer->MakeWritable(cmd);
	}

	if (firstRun)
	{
		if (probeRenderer->IsUsable())
		{
			probePool = probeRenderer->CreateDescriptorPool(static_cast<uint32>(materials.size()));
			const Descriptor &diffuseDescriptor = probeRenderer->GetDiffuseDescriptor();

			for (const PumMaterial &mat : stageMaterials)
			{
				probeSets.emplace_back(std::move(probePool->Allocate()));
				if (mat.HasDiffuseTexture) probeSets.back().Write(diffuseDescriptor, *textures[mat.DiffuseTexture]);
				else probeSets.back().Write(diffuseDescriptor, *textures[textures.size() - 3]);
			}
		}
		else return;

		firstRun = false;
		cmd.CopyEntireBuffer(*stagingBuffer, *vrtxBuffer);
		cmd.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		cam->SetEnvironment(environment->GetTexture());
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

			if (mat.HasEmissiveTexture) mat2.SetEmissive(*textures[mat.EmissiveTexture]);
			else mat2.SetEmissive(*textures[textures.size() - 2]);

			if (mat.HasOcclusionTexture) mat2.SetOcclusion(*textures[mat.OcclusionTexture]);
			else mat2.SetOcclusion(*textures[textures.size() - 3]);

			mat2.Update(cmd);
		}

		Material &defMat = *materials.back();
		defMat.ForceUpdate();
		defMat.SetDiffuse(*textures[textures.size() - 3]);
		defMat.SetSpecular(*textures[textures.size() - 2]);
		defMat.SetNormal(*textures[textures.size() - 1]);
		defMat.SetEmissive(*textures[textures.size() - 2]);
		defMat.SetOcclusion(*textures[textures.size() - 3]);
		defMat.Update(cmd);

		Profiler::End();
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

	vector<Matrix> nodeTransforms{ nodes.size() };
	if (!nodeTransforms.empty()) SetNodeTransform(nodeTransforms, 0, mdlMtrx);

	uint32 drawCalls = 0, batchCalls = 0;
	Profiler::Add("Light Probe Update", Color::Green(), queries->GetTimeDelta(0, false) * 0.001f);
	Profiler::Add("Render", Color::Yellow(), queries->GetTimeDelta(2, false) * 0.001f);
	cmd.ResetQueries(*queries);

	Profiler::Begin("Light Probe Update", Color::Green());
	probeRenderer->Start(*environment, cmd);
	cmd.WriteTimestamp(PipelineStageFlag::TopOfPipe, *queries, 0);
	size_t i = 0;
	for (const auto[matIdx, mesh] : meshes)
	{
		const Matrix transform = GetMeshTransform(nodeTransforms, i++);

		//if (environment->Cull(mesh->GetBoundingBox() * mdlMtrx)) continue;
		probeRenderer->Render(*mesh, matIdx != -1 ? probeSets[matIdx] : probeSets.back(), transform, cmd);
		++drawCalls;
		++batchCalls;
	}
	i = 0;
	cmd.WriteTimestamp(PipelineStageFlag::BottomOfPipe, *queries, 1);
	probeRenderer->End(*environment, cmd);
	Profiler::End();

	cam->Update(dt * updateCam, cmd);
	light->Update(cmd);

	Profiler::Begin("Render", Color::Yellow());
	cmd.BeginRenderPass(*renderPass, GetWindow().GetCurrentFramebuffer(*renderPass), SubpassContents::Inline);
	cmd.BindGraphicsPipeline(*gfxPipeline);

	cmd.BindGraphicsDescriptor(*gfxPipeline, *cam);
	cmd.BindGraphicsDescriptor(*gfxPipeline, *light);

	cmd.WriteTimestamp(PipelineStageFlag::TopOfPipe, *queries, 2);
	cmd.AddLabel("Model", Color::Blue());
	uint32 oldMat = -1;

	for (const auto[matIdx, mesh] : meshes)
	{
		const Matrix transform = GetMeshTransform(nodeTransforms, i++);
		if (cam->GetClip().IntersectionBox(mesh->GetBoundingBox() * transform))
		{
			cmd.PushConstants(*gfxPipeline, ShaderStageFlag::Vertex, 0, sizeof(Matrix), transform.GetComponents());
			//dbgRenderer->AddBox(mesh->GetBoundingBox(), transform, Color::Yellow());

			++drawCalls;
			if (matIdx != oldMat)
			{
				const Material &material = *(matIdx != -1 ? materials[matIdx] : materials.back());
				cmd.BindGraphicsDescriptor(*gfxPipeline, material);
				oldMat = matIdx;
				++batchCalls;
			}

			mesh->Bind(cmd, 0);
			mesh->Draw(cmd);
		}
	}

	cmd.EndLabel();
	cmd.WriteTimestamp(PipelineStageFlag::BottomOfPipe, *queries, 3);
	cmd.EndRenderPass();
	Profiler::End();

	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Text("Draw Calls: %u (%u batches)", drawCalls, batchCalls);
		ImGui::Separator();
		ImGui::Text("%d FPS", iround(recip(dt)));

		DeviceSize used;
		if (GetDevice().GetPhysicalDevice().TryGetUsedDeviceLocalBytes(used))
		{
			DeviceSize max = GetDevice().GetPhysicalDevice().GetDeviceLocalBytes();

			ImGui::Separator();
			ImGui::Text("GPU memory %u/%u", b2mb(used), b2mb(max));
		}

		ImGui::EndMainMenuBar();
	}

	dbgRenderer->Render(cmd, cam->GetProjection(), cam->GetView());


	Profiler::Visualize();
}

void TestGame::SetNodeTransform(vector<Matrix>& transforms, size_t idx, const Matrix & parent) const
{
	const PumNode &node = nodes[idx];
	const Matrix transform = parent * node.GetTransform();
	transforms[idx] = transform;

	for (size_t child : node.Children)
	{
		SetNodeTransform(transforms, child, transform);
	}
}

Matrix TestGame::GetMeshTransform(vector<Matrix>& nodeTransforms, size_t idx) const
{
	if (nodeTransforms.empty()) return Matrix{};

	for (size_t i = 0; i < nodes.size(); i++)
	{
		const PumNode &node = nodes[i];
		if (node.HasMesh && node.Mesh == idx) return nodeTransforms[i];
	}

	return nodeTransforms.front();
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

void TestGame::OnSwapchainRecreated(const Pu::GameWindow&, const SwapchainReCreatedEventArgs & args)
{
	if (renderPass && renderPass->IsLoaded())
	{
		if (args.FormatChanged) renderPass->Recreate();
		if (args.AreaChanged)
		{
			if (depthBuffer)
			{
				CreateDepthBuffer();
				dbgRenderer->Reset(*depthBuffer);
			}

			CreateGraphicsPipeline();
		}
	}
}

void TestGame::CreateDepthBuffer(void)
{
	if (depthBuffer) delete depthBuffer;
	depthBuffer = new DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetSize());
	markDepthBuffer = true;

	if (!dbgRenderer) dbgRenderer = new DebugRenderer(GetWindow(), GetContent(), depthBuffer, 2.0f);
}

void TestGame::InitializeRenderpass(Pu::Renderpass&)
{
	if (!depthBuffer) CreateDepthBuffer();
	Subpass &pass = renderPass->GetSubpass(0);

	pass.GetOutput("L0").SetDescription(GetWindow().GetSwapchain());
	pass.AddDepthStencil().SetDescription(*depthBuffer);

	pass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
	pass.GetAttribute("Tangent").SetOffset(vkoffsetof(SkinnedAnimated, Tangent));
	pass.GetAttribute("TexCoord").SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));
}

void TestGame::FinalizeRenderpass(Pu::Renderpass&)
{
	const Subpass &pass = renderPass->GetSubpass(0);
	CreateGraphicsPipeline();

	descPoolCam = new DescriptorPool(*gfxPipeline, pass, 0, 1);
	cam = new FreeCamera(GetWindow().GetNative(), *descPoolCam, GetInput());
	cam->Move(0.0f, 1.0f, -1.0f);
	cam->Yaw = PI2;

	descPoolLight = new DescriptorPool(*gfxPipeline, pass, 2, 1);
	light = new DirectionalLight(*descPoolLight);

	descPoolMats = new DescriptorPool(*gfxPipeline, pass, 1, stageMaterials.size() + 1);
	for (PumMaterial &material : stageMaterials)
	{
		materials.emplace_back(new Material(*descPoolMats, material));
	}

	materials.emplace_back(new Material(*descPoolMats));
}

void TestGame::CreateGraphicsPipeline(void)
{
	if (gfxPipeline) delete gfxPipeline;

	gfxPipeline = new GraphicsPipeline(*renderPass, 0);
	gfxPipeline->SetViewport(GetWindow().GetNative().GetClientBounds());
	gfxPipeline->SetTopology(PrimitiveTopology::TriangleList);
	gfxPipeline->EnableDepthTest(true, CompareOp::LessOrEqual);
	gfxPipeline->AddVertexBinding(0, meshes.front().second->GetStride());

	GetWindow().CreateFramebuffers(*renderPass, { &depthBuffer->GetView() });
	gfxPipeline->Finalize();
}