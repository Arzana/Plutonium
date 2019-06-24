#include "TestGame.h"
#include <Input/Keys.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Graphics/Textures/DepthBuffer.h>
#include <Core/Diagnostics/CPU.h>
#include <imgui.h>
#include <Streams/FileReader.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 2560.0f, 1440.0f, 2)
{
	GetInput().AnyKeyDown += [this](const InputDevice &sender, const ButtonEventArgs args)
	{
		if (sender.Type == InputDeviceType::Keyboard)
		{
			switch (args.KeyCode)
			{
			case (_CrtEnum2Int(Keys::Escape)):
				Exit();
				break;
			case (_CrtEnum2Int(Keys::Up)):
				cam->MoveSpeed++;
				break;
			case (_CrtEnum2Int(Keys::Down)):
				cam->MoveSpeed--;
				break;
			case (_CrtEnum2Int(Keys::M)):
				if (Mouse::IsCursorVisible())
				{
					Mouse::HideCursor();
					cam->Enable();
				}
				else
				{
					Mouse::ShowCursor();
					cam->Disable();
				}
				break;
			}
		}
		else if (sender.Type == InputDeviceType::GamePad)
		{
			switch (args.KeyCode)
			{
			case (_CrtEnum2Int(Keys::XBoxB)):
				Exit();
				break;
			}
		}
	};
}

void TestGame::Initialize(void)
{
	/* Make sure that the cursor is hiden and locked if the window is focused. */
	GetWindow().GetNative().OnGainedFocus += [](const NativeWindow &sender)
	{
		Mouse::HideCursor();
		Mouse::LockCursor(sender);
	};

	GetWindow().GetNative().OnLostFocus += [](const NativeWindow&)
	{
		Mouse::ShowCursor();
		Mouse::FreeCursor();
	};

	AddComponent(cam = new FreeCamera(*this, GetInput()));
	depthBuffer = new DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetNative().GetSize());
	queryPool = new QueryPool(GetDevice(), QueryType::Timestamp, 2);
	debugRenderer = new DebugRenderer(GetWindow(), GetContent(), depthBuffer, 2.0f);

	/* Setup and load the renderpass. */
	renderpass = &GetContent().FetchRenderpass({ { L"{Shaders}SkinnedAnimated.vert.spv", L"{Shaders}SkinnedAnimated.frag.spv" } });
	renderpass->PreCreate += [this](Renderpass&)
	{
		Subpass &subpass = renderpass->GetSubpass(0);
		
		subpass.GetOutput("FragColor").SetDescription(GetWindow().GetSwapchain());
		subpass.AddDepthStencil().SetDescription(*depthBuffer);

		subpass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
		subpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));
		subpass.GetAttribute("Joints").SetOffset(vkoffsetof(SkinnedAnimated, Joints));
		subpass.GetAttribute("Weights").SetOffset(vkoffsetof(SkinnedAnimated, Weights));
	};

	/* Setup the graphics pipeline and the descriptors. */
	renderpass->PostCreate += [this](Renderpass&)
	{
		/* Create the descirptor pool and uniforms. */
		descriptorPool = new DescriptorPool(*renderpass, 2);
		material = new MonsterMaterial(renderpass->GetSubpass(0), *descriptorPool);
		transform = new TransformBlock(renderpass->GetSubpass(0), *descriptorPool);

		pipeline = new GraphicsPipeline(*renderpass, 0);
		pipeline->SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline->SetTopology(PrimitiveTopology::TriangleList);
		pipeline->EnableDepthTest(true, CompareOp::LessOrEqual);
		pipeline->AddVertexBinding<SkinnedAnimated>(0);

		GetWindow().CreateFrameBuffers(*renderpass, { &depthBuffer->GetView() });
		pipeline->Finalize();
	};

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().SwapchainRecreated += [this](const GameWindow&)
	{
		GetWindow().CreateFrameBuffers(*renderpass, { &depthBuffer->GetView() });
	};
}

void TestGame::LoadContent(void)
{
	FileReader file(L"assets/Models/Monster.pum");
	const string raw = file.ReadToEnd();
	BinaryReader reader(raw.data(), raw.length(), Endian::Little);

	PuMData pum(GetDevice(), reader);
	PumMesh &geometry = pum.Geometry.front();
	rawMaterial = pum.Materials[geometry.Material];

	vrtxBuffer = new Buffer(GetDevice(), pum.Buffer->GetSize(), BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer | BufferUsageFlag::TransferDst, false);
	vrtxStagingBuffer = pum.Buffer;

	mesh = Mesh(*vrtxBuffer, geometry);
	bb = geometry.Bounds;
	image = &GetContent().FetchTexture2D(pum.Textures[rawMaterial.DiffuseTexture].Path.toWide(), pum.Textures[rawMaterial.DiffuseTexture].GetSamplerCreateInfo(), false);
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);

	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	delete material;
	delete transform;
	delete descriptorPool;
	delete pipeline;
	delete debugRenderer;
	delete depthBuffer;
	delete queryPool;

	GetContent().Release(*renderpass);
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

void TestGame::Render(float dt, CommandBuffer & cmdBuffer)
{
	if (!transform || !pipeline->IsUsable()) return;

	static bool firstRender = true;
	if (firstRender)
	{
		/* Wait for the graphics pipeline to be usable. */
		if (!image->IsUsable()) return;
		firstRender = false;

		/* Initialize the depth buffer for writing. */
		depthBuffer->MakeWritable(cmdBuffer);

		/* Copy model to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		/* Make sure the image layout is suitable for shader reads. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, image->GetFullRange());

		/* Update the descriptor. */
		material->SetParameters(rawMaterial, *image);
	}
	else	// Render ImGui
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::BeginCombo("##ColorSpace", to_string(GetWindow().GetSwapchain().GetColorSpace())))
				{
					for (const SurfaceFormat &format : GetWindow().GetSupportedFormats())
					{
						if (ImGui::Selectable(((string)format).c_str())) GetWindow().SetColorSpace(format.ColorSpace);
					}

					ImGui::EndCombo();
				}

				ImGui::EndMenu();
			}

			ImGui::Text("FPS: %d (%f ms)", iround(1.0f / dt), queryPool->GetTimeDelta(0, false) * 0.000001f);
			ImGui::Text("CPU: %.0f%%", CPU::GetCurrentProcessUsage() * 100.0f);
			ImGui::EndMainMenuBar();
		}
	}

	/* Update the descriptor if needed. */
	transform->Update(cmdBuffer);
	material->Update(cmdBuffer);

	/* Timestamps. */
	cmdBuffer.WriteTimestamp(PipelineStageFlag::TopOfPipe, *queryPool, 0);
	cmdBuffer.WriteTimestamp(PipelineStageFlag::BottomOfPipe, *queryPool, 1);

	/* Render scene. */
	cmdBuffer.BeginRenderPass(*renderpass, GetWindow().GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);

	cmdBuffer.AddLabel(u8"Monster", Color::Lime());
	cmdBuffer.BindGraphicsDescriptor(*transform);
	cmdBuffer.BindGraphicsDescriptor(*material);
	mesh.Bind(cmdBuffer, 0);
	mesh.Draw(cmdBuffer);
	cmdBuffer.EndLabel();

	cmdBuffer.EndRenderPass();

	debugRenderer->AddBox(bb, Color::Red());
	debugRenderer->Render(cmdBuffer, cam->GetProjection(), cam->GetView());
}