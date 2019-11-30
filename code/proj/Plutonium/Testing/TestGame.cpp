#include "TestGame.h"
#include <Input/Keys.h>
#include <Streams/FileReader.h>
#include <Core/Diagnostics/Stopwatch.h>
#include <Graphics/VertexLayouts/Basic3D.h>

using namespace Pu;

Stopwatch sw;

TestGame::TestGame(void)
	: Application(L"TestGame", 1280.0f, 720.0f, std::thread::hardware_concurrency() - 2), cam(nullptr),
	renderPass(nullptr), gfxPipeline(nullptr), depthBuffer(nullptr),
	descPoolCam(nullptr), descPoolMats(nullptr), vrtxBuffer(nullptr),
	stagingBuffer(nullptr), transform(nullptr), firstRun(true),
	markDepthBuffer(true)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
	mdlMtrx = Matrix::CreateRoll(PI) * Matrix::CreateScalar(0.03f);
}

void TestGame::Initialize(void)
{
	GetWindow().GetNative().SetMode(WindowMode::Borderless);
	AddComponent(cam = new FreeCamera(*this, GetInput()));
	cam->Move(0.0f, -5.0f, -3.0f);
	cam->Yaw = PI2;
	GetWindow().SwapchainRecreated.Add(*this, &TestGame::OnSwapchainRecreated);
}

void TestGame::LoadContent(void)
{
	sw.Start();
	const string file = FileReader(L"assets/Models/Sponza.pum").ReadToEnd();
	BinaryReader reader{ file.c_str(), file.length(), Endian::Little };
	PuMData mdl{ GetDevice(), reader };

	stagingBuffer = mdl.Buffer;
	vrtxBuffer = new Buffer(GetDevice(), stagingBuffer->GetSize(), BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer, false);
	stageMaterials = std::move(mdl.Materials);

	for (const PumMesh &mesh : mdl.Geometry)
	{
		if (mesh.HasNormals && mesh.HasTangents && mesh.HasTextureCoordinates)
		{
			if (mesh.HasMaterial)
			{
				meshes.emplace_back(std::make_tuple(mesh.Material, new Mesh(*vrtxBuffer, mesh)));
			}
			else meshes.emplace_back(std::make_tuple(-1, new Mesh(*vrtxBuffer, mesh)));
		}
	}

	AssetFetcher &fetcher = GetContent();
	for (const PumTexture &texture : mdl.Textures)
	{
		textures.emplace_back(&fetcher.FetchTexture2D(texture.Path.toWide(), texture.GetSamplerCreateInfo(), true));
	}

	textures.emplace_back(&fetcher.CreateTexture2D("DefaultDiffuse", Color::White().ToArray(), 1, 1, SamplerCreateInfo()));
	textures.emplace_back(&fetcher.CreateTexture2D("DefaultSpecularGlossiness", Color::Black().ToArray(), 1, 1, SamplerCreateInfo()));
	textures.emplace_back(&fetcher.CreateTexture2D("DefaultNormal", Color::Malibu().ToArray(), 1, 1, SamplerCreateInfo()));
	stageMaterials.emplace_back(PumMaterial());

	renderPass = &GetContent().FetchRenderpass({ { L"{Shaders}Basic3D.vert.spv", L"{Shaders}Basic3D.frag.spv" } });
	renderPass->PreCreate.Add(*this, &TestGame::InitializeRenderpass);
	renderPass->PostCreate.Add(*this, &TestGame::FinalizeRenderpass);
}

void TestGame::UnLoadContent(void)
{
	AssetFetcher &fetcher = GetContent();

	if (transform) delete transform;
	for (Material *material : materials) delete material;
	if (descPoolCam) delete descPoolCam;
	if (descPoolMats) delete descPoolMats;
	if (gfxPipeline) delete gfxPipeline;

	delete vrtxBuffer;
	delete stagingBuffer;
	fetcher.Release(*renderPass);

	for (auto[mat, mesh] : meshes) delete mesh;
	for (Texture2D *texture : textures) fetcher.Release(*texture);
}

void TestGame::Finalize(void)
{
	if (depthBuffer) delete depthBuffer;
}

void TestGame::Update(float)
{
	if (transform)
	{
		transform->SetProjection(cam->GetProjection());
		transform->SetView(cam->GetView());
		transform->SetCamPos(cam->GetPosition());
	}
}

void TestGame::Render(float, CommandBuffer &cmd)
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
		firstRun = false;
		cmd.CopyEntireBuffer(*stagingBuffer, *vrtxBuffer);
		cmd.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		vector<std::tuple<const Image*, ImageSubresourceRange>> tmp;
		tmp.reserve(textures.size());
		for (const Texture2D *texture : textures) tmp.emplace_back(std::make_tuple(&texture->GetImage(), texture->GetFullRange()));
		cmd.MemoryBarrier(tmp, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead);

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

			mat2.Update(cmd);
		}

		sw.End();
		Log::Message("Finished loading, took %f seconds.", sw.SecondsAccurate());
	}

	transform->Update(cmd);
	cmd.BeginRenderPass(*renderPass, GetWindow().GetCurrentFramebuffer(*renderPass), SubpassContents::Inline);
	cmd.BindGraphicsPipeline(*gfxPipeline);

	cmd.BindGraphicsDescriptor(*transform);
	cmd.PushConstants(*renderPass, ShaderStageFlag::Vertex, sizeof(Matrix), mdlMtrx.GetComponents());

	for (const auto[matIdx, mesh] : meshes)
	{
		cmd.BindGraphicsDescriptor(matIdx != -1 ? *materials[matIdx] : *materials.back());
		mesh->Bind(cmd, 0);
		mesh->Draw(cmd);
	}

	cmd.EndRenderPass();
}

void TestGame::OnAnyKeyDown(const InputDevice & sender, const ButtonEventArgs &args)
{
	if (sender.Type == InputDeviceType::Keyboard)
	{
		if (args.KeyCode == _CrtEnum2Int(Keys::Escape)) Exit();
		else if (args.KeyCode == _CrtEnum2Int(Keys::C))
		{
			if (cam->IsEnabled()) cam->Disable();
			else cam->Enable();
		}
		else if (args.KeyCode == _CrtEnum2Int(Keys::NumAdd)) cam->MoveSpeed++;
		else if (args.KeyCode == _CrtEnum2Int(Keys::NumSubtract)) cam->MoveSpeed--;
	}
	else if (sender.Type == InputDeviceType::GamePad)
	{
		if (args.KeyCode == _CrtEnum2Int(Keys::XBoxB)) Exit();
	}
}

void TestGame::OnSwapchainRecreated(const Pu::GameWindow &)
{
	if (depthBuffer) CreateDepthBuffer();
	if (renderPass->IsLoaded()) CreateGraphicsPipeline();
}

void TestGame::CreateDepthBuffer(void)
{
	if (depthBuffer) delete depthBuffer;
	depthBuffer = new DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetSize());
}

void TestGame::InitializeRenderpass(Pu::Renderpass&)
{
	if (!depthBuffer) CreateDepthBuffer();
	Subpass &pass = renderPass->GetSubpass(0);

	pass.GetOutput("L0").SetDescription(GetWindow().GetSwapchain());
	pass.AddDepthStencil().SetDescription(*depthBuffer);

	pass.GetAttribute("Normal").SetOffset(vkoffsetof(Basic3D, Normal));
	pass.GetAttribute("Tangent").SetOffset(vkoffsetof(Basic3D, Tangent));
	pass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Basic3D, TexCoord));
}

void TestGame::FinalizeRenderpass(Pu::Renderpass&)
{
	const Subpass &pass = renderPass->GetSubpass(0);

	descPoolCam = new DescriptorPool(*renderPass, pass, 0, 1);
	transform = new TransformBlock(*descPoolCam);

	descPoolMats = new DescriptorPool(*renderPass, pass, 1, (stageMaterials.size() << 1) + 1);
	for (const PumMaterial &material : stageMaterials)
	{
		materials.emplace_back(new Material(*descPoolMats, material));
	}

	materials.emplace_back(new Material(*descPoolMats));
	materials.back()->SetDiffuse(Color::Red());
	CreateGraphicsPipeline();
}

void TestGame::CreateGraphicsPipeline(void)
{
	if (gfxPipeline) delete gfxPipeline;

	gfxPipeline = new GraphicsPipeline(*renderPass, 0);
	gfxPipeline->SetViewport(GetWindow().GetNative().GetClientBounds());
	gfxPipeline->SetTopology(PrimitiveTopology::TriangleList);
	gfxPipeline->EnableDepthTest(true, CompareOp::LessOrEqual);
	gfxPipeline->SetCullMode(CullModeFlag::Back);
	gfxPipeline->AddVertexBinding<Basic3D>(0);

	GetWindow().CreateFrameBuffers(*renderPass, { &depthBuffer->GetView() });
	gfxPipeline->Finalize();
}