#include "TestGame.h"
#include <Environment/Terrain/TerrainCreator.h>
#include <Graphics/VertexLayouts/Terrain.h>
#include <Graphics/Textures/DepthBuffer.h>
#include <Core/Diagnostics/CPU.h>
#include <Input/Keys.h>
#include <imgui.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 1280.0f, 720.0f, 2), remarkDepthBuffer(true)
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
	newMode = GetWindow().GetNative().GetWindowMode();

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
	timestamps = new QueryPool(GetDevice(), QueryType::Timestamp, 2);

	/* Setup and load the renderpass. */
	renderpass = &GetContent().FetchRenderpass({ { L"{Shaders}Terrain.vert.spv", L"{Shaders}Terrain.frag.spv" } });
	renderpass->PreCreate.Add(*this, &TestGame::RenderpassPreCreate);
	renderpass->PostCreate.Add(*this, &TestGame::RenderpassPostCreate);

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().SwapchainRecreated += [this](const GameWindow&)
	{
		*depthBuffer = DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetNative().GetSize());

		if (renderpass->IsLoaded())
		{
			delete pipeline;
			delete material;
			delete transform;
			delete descriptorPool;
			GetContent().Release(*renderpass);

			pipeline = nullptr;
			material = nullptr;
			transform = nullptr;
			descriptorPool = nullptr;

			renderpass = &GetContent().FetchRenderpass({ { L"{Shaders}Terrain.vert.spv", L"{Shaders}Terrain.frag.spv" } });
			renderpass->PreCreate.Add(*this, &TestGame::RenderpassPreCreate);
			renderpass->PostCreate.Add(*this, &TestGame::RenderpassPostCreate);
			if (renderpass->IsLoaded()) RenderpassPostCreate(*renderpass);
		}
	};
}

void TestGame::LoadContent(void)
{
	Log::Warning("Generating Lithosphere, this may take a while.");
	TectonicSettings settings;
	settings.PlateCount = 5;

	lithosphere = new Lithosphere(65, settings);
}

void TestGame::UnLoadContent(void)
{
	delete vrtxStagingBuffer;
	delete vrtxBuffer;
	delete lithosphere;
}

void TestGame::Finalize(void)
{
	delete material;
	delete transform;
	delete descriptorPool;
	delete pipeline;
	delete depthBuffer;
	delete timestamps;

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

	/* This cannot happen in the render method because it would require resources to change. */
	if (newMode != GetWindow().GetNative().GetWindowMode())
	{
		remarkDepthBuffer = true;
		GetWindow().GetNative().SetMode(newMode);
	}
}

void TestGame::Render(float dt, CommandBuffer & cmdBuffer)
{
	/* Wait for the graphics pipeline and image to be usable. */
	if (!pipeline || !pipeline->IsUsable()) return;

	// Render ImGui
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::BeginCombo("##Mode", "Window mode"))
			{
				if (ImGui::Selectable("Windowed")) newMode = WindowMode::Windowed;
				if (ImGui::Selectable("Borderless")) newMode = WindowMode::Borderless;
				if (ImGui::Selectable("Fullscreen")) newMode = WindowMode::Fullscreen;
				ImGui::EndCombo();
			}

			ImGui::EndMenu();
		}

		if (ImGui::Button("Update"))
		{
			/* Wait for all resources to become available again. */
			GetDevice().WaitIdle();
			if (vrtxBuffer)
			{
				delete vrtxBuffer;
				delete vrtxStagingBuffer;
				lithosphere->Update();
			}

			/* Create the buffers and the mesh. */
			TerrainCreator creator{ *lithosphere };
			vrtxBuffer = new Buffer(GetDevice(), creator.GetBufferSize(), BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer | BufferUsageFlag::TransferDst, false);
			vrtxStagingBuffer = new StagingBuffer(*vrtxBuffer);
			mesh = creator.Generate(*vrtxStagingBuffer, *vrtxBuffer);

			/* Copy model to final vertex buffer. */
			cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
			cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
		}

		ImGui::Text("Cycle: %zu/%zu, Iteration: %zu", lithosphere->GetCycleCount(), lithosphere->GetSettings().Cycles, lithosphere->GetIterationCount());

		ImGui::Text("FPS: %d (%f ms)", iround(1.0f / dt), timestamps->GetTimeDelta(0, true) * 0.000001f);
		ImGui::Text("CPU: %.0f%%", CPU::GetCurrentProcessUsage() * 100.0f);
		ImGui::EndMainMenuBar();
	}

	if (remarkDepthBuffer)
	{
		remarkDepthBuffer = false;
		depthBuffer->MakeWritable(cmdBuffer);
		material->SetParameters(0.0f, Color::Black(), Color::White());
	}

	if (vrtxBuffer)
	{
		/* Update the descriptor if needed. */
		transform->Update(cmdBuffer);
		material->Update(cmdBuffer);

		/* Timestamps. */
		cmdBuffer.WriteTimestamp(PipelineStageFlag::TopOfPipe, *timestamps, 0);
		cmdBuffer.WriteTimestamp(PipelineStageFlag::BottomOfPipe, *timestamps, 1);

		/* Render scene. */
		cmdBuffer.BeginRenderPass(*renderpass, GetWindow().GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
		cmdBuffer.BindGraphicsPipeline(*pipeline);

		cmdBuffer.AddLabel(u8"Terrain", Color::Lime());
		cmdBuffer.BindGraphicsDescriptor(*transform);
		cmdBuffer.BindGraphicsDescriptor(*material);
		mesh.Bind(cmdBuffer, 0);
		mesh.Draw(cmdBuffer);
		cmdBuffer.EndLabel();

		cmdBuffer.EndRenderPass();
	}
}

void TestGame::RenderpassPreCreate(Pu::Renderpass &)
{
	Subpass &subpass = renderpass->GetSubpass(0);

	subpass.GetOutput("L0").SetDescription(GetWindow().GetSwapchain());
	subpass.AddDepthStencil().SetDescription(*depthBuffer);

	subpass.GetAttribute("Normal").SetOffset(vkoffsetof(Terrain, Normal));
	subpass.GetAttribute("PlateId").SetOffset(vkoffsetof(Terrain, PlateId));
}

void TestGame::RenderpassPostCreate(Pu::Renderpass &)
{
	/* Create the descirptor pool and uniforms. */
	descriptorPool = new DescriptorPool(*renderpass, 2);
	material = new Material(renderpass->GetSubpass(0), *descriptorPool);
	transform = new TransformBlock(renderpass->GetSubpass(0), *descriptorPool);

	pipeline = new GraphicsPipeline(*renderpass, 0);
	pipeline->SetViewport(GetWindow().GetNative().GetClientBounds());
	pipeline->SetTopology(PrimitiveTopology::TriangleList);
	pipeline->EnableDepthTest(true, CompareOp::LessOrEqual);
	pipeline->SetCullMode(CullModeFlag::Back);
	pipeline->AddVertexBinding<Terrain>(0);

	GetWindow().CreateFrameBuffers(*renderpass, { &depthBuffer->GetView() });
	pipeline->Finalize();
}