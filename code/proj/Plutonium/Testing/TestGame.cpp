#include "TestGame.h"
#include <Input/Keys.h>
#include <Graphics/Models/ShapeCreator.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 1280.0f, 720.0f, 2), cam(nullptr),
	renderPass(nullptr), gfxPipeline(nullptr), depthBuffer(nullptr),
	descPool(nullptr), vrtxBuffer(nullptr), stagingBuffer(nullptr),
	material(nullptr), transform(nullptr), firstRun(true), markDepthBuffer(true)
{
	GetInput().AnyKeyDown.Add(*this, &TestGame::OnAnyKeyDown);
}

void TestGame::Initialize(void)
{
	AddComponent(cam = new FreeCamera(*this, GetInput()));
	GetWindow().SwapchainRecreated.Add(*this, &TestGame::OnSwapchainRecreated);
}

void TestGame::LoadContent(void)
{
	renderPass = &GetContent().FetchRenderpass({ { L"{Shaders}Basic3D.vert.spv", L"{Shaders}Basic3D.frag.spv" } });
	renderPass->PreCreate.Add(*this, &TestGame::InitializeRenderpass);
	renderPass->PostCreate.Add(*this, &TestGame::FinalizeRenderpass);

	vrtxBuffer = new Buffer(GetDevice(), ShapeCreator::GetSphereBufferSize(32), BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer, false);
	stagingBuffer = new StagingBuffer(*vrtxBuffer);
	mesh = ShapeCreator::Sphere(*stagingBuffer, *vrtxBuffer, 32);
}

void TestGame::UnLoadContent(void)
{
	if (material) delete material;
	if (transform) delete transform;
	if (descPool) delete descPool;
	if (gfxPipeline) delete gfxPipeline;

	delete vrtxBuffer;
	delete stagingBuffer;
	GetContent().Release(*renderPass);
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
	}

	if (ImGui::Begin("Material Editor"))
	{
		float v = material->GetGlossiness();
		if (ImGui::SliderFloat("Glossiness", &v, 0.0f, 1.0f))
		{
			material->SetGlossiness(v);
		}

		v = material->GetSpecularPower();
		if (ImGui::SliderFloat("Specular Power", &v, 0.0f, 10.0f))
		{
			material->SetSpecularPower(v);
		}

		Vector3 tint = material->GetSpecularColor();
		if (ImGui::ColorPicker3("F0", tint.f))
		{
			material->SetSpecDiffuse(tint);
		}

		if (ImGui::BeginCombo("##Preset", "Preset"))
		{
			if (ImGui::Selectable("Iron")) material->SetSpecDiffuse(Vector3(0.56f, 0.57f, 0.58f));
			if (ImGui::Selectable("Gold")) material->SetSpecDiffuse(Vector3(1.0f, 0.71f, 0.29f));
			if (ImGui::Selectable("Copper")) material->SetSpecDiffuse(Vector3(0.95f, 0.64f, 0.54f));
			ImGui::EndCombo();
		}

		ImGui::End();
	}

	transform->Update(cmd);
	material->Update(cmd);

	cmd.BeginRenderPass(*renderPass, GetWindow().GetCurrentFramebuffer(*renderPass), SubpassContents::Inline);
	cmd.BindGraphicsPipeline(*gfxPipeline);

	cmd.BindGraphicsDescriptor(*transform);
	cmd.BindGraphicsDescriptor(*material);
	mesh.Bind(cmd, 0);
	mesh.Draw(cmd);

	cmd.EndRenderPass();
}

void TestGame::OnAnyKeyDown(const InputDevice & sender, const ButtonEventArgs &args)
{
	if (sender.Type == InputDeviceType::Keyboard)
	{
		if (args.KeyCode == _CrtEnum2Int(Keys::Escape)) Exit();
		if (args.KeyCode == _CrtEnum2Int(Keys::C))
		{
			if (cam->IsEnabled()) cam->Disable();
			else cam->Enable();
		}
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
	pass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Basic3D, TexCoord));
}

void TestGame::FinalizeRenderpass(Pu::Renderpass&)
{
	const Subpass &pass = renderPass->GetSubpass(0);

	descPool = new DescriptorPool(*renderPass, 2);
	material = new Material(pass, *descPool);
	transform = new TransformBlock(pass, *descPool);

	const Color fe{ 0.77f, 0.78f, 0.78f };
	material->SetParameters(0.5f, 2.0f, fe, fe);

	transform->SetModel(Matrix::CreateTranslation(-1.0f, 1.0f, 5.0f));

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