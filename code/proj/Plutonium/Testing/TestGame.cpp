#include "TestGame.h"
#include <Input/Keys.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Graphics/Textures/DepthBuffer.h>
#include <Core/Diagnostics/CPU.h>
#include <imgui.h>

#include <Streams/FileReader.h>
#include <Content/PumLoader.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 1920.0f, 1080.0f)
{
	GetInput().AnyKeyboard.KeyDown += [this](const Keyboard&, uint16 key)
	{
		switch (key)
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

	AddComponent(cam = new FreeCamera(*this, GetInput().AnyKeyboard, GetInput().AnyMouse));
	depthBuffer = new DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetNative().GetSize());
	queryPool = new QueryPool(GetDevice(), QueryType::Timestamp, 2);

	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice(), 2);
	pipeline->PostInitialize += [this](GraphicsPipeline &pipeline)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleList);
		pipeline.SetDepthStencilMode(true, true, false);
		pipeline.AddVertexBinding(0, sizeof(SkinnedAnimated));
		pipeline.Finalize();

		/* Create the framebuffers and the required uniform block. */
		GetWindow().CreateFrameBuffers(pipeline.GetRenderpass(), { &depthBuffer->GetView() });
		transform = new TransformBlock(GetDevice(), pipeline);
	};

	/* Setup and load render pass. */
	GetContent().FetchRenderpass(*pipeline, { L"{Shaders}SkinnedAnimated.vert.spv", L"{Shaders}SkinnedAnimated.frag.spv" }).OnLinkCompleted += [this](Renderpass &renderpass)
	{
		/* Set description and layout of FragColor. */
		renderpass.GetOutput("FragColor").SetDescription(GetWindow().GetSwapchain());

		/* Set the description and layout of the depth buffer. */
		renderpass.AddDepthStencil().SetDescription(*depthBuffer);

		/* Set offset for uv attribute (position is default). */
		renderpass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
		renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));
		renderpass.GetAttribute("Joints").SetOffset(vkoffsetof(SkinnedAnimated, Joints));
		renderpass.GetAttribute("Weights").SetOffset(vkoffsetof(SkinnedAnimated, Weights));
	};

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().GetNative().OnSizeChanged += [this](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		GetWindow().CreateFrameBuffers(pipeline->GetRenderpass());
	};
}

void TestGame::LoadContent(void)
{
	FileReader file(L"assets/Models/Monster.pum");
	const string raw = file.ReadToEnd();
	BinaryReader reader(raw.data(), raw.length(), Endian::Little);
	PuMData pum(GetDevice(), reader);
	PumMesh &geometry = pum.Geometry.front();

	vrtxBuffer = new Buffer(GetDevice(), pum.Buffer->GetSize(), BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer | BufferUsageFlag::TransferDst, false);
	mesh = new BufferView(*vrtxBuffer, geometry.VertexViewStart, geometry.VertexViewSize, sizeof(Vector3) * 2 + sizeof(Vector2));
	index = new BufferView(*vrtxBuffer, geometry.IndexViewStart, geometry.IndexViewSize, sizeof(uint16));
	vrtxStagingBuffer = pum.Buffer;

	image = &GetContent().FetchTexture2D(pum.Textures.front().Path.toWide(), pum.Textures.front().GetSamplerCreateInfo(), false);
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);

	delete index;
	delete mesh;
	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	GetContent().Release(*pipeline);

	delete material;
	delete transform;
	delete pipeline;
	delete depthBuffer;
	delete queryPool;
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
	if (!transform) return;
	/* Update the descriptor if needed. */
	transform->Update(cmdBuffer);

	static bool firstRender = true;
	if (firstRender)
	{
		/* Wait for the graphics pipeline to be usable. */
		if (!image->IsUsable()) return;
		firstRender = false;

		/* Initialize the depth buffer for writing. */
		depthBuffer->MakeWritable(cmdBuffer);

		/* Copy quad to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		/* Make sure the image layout is suitable for shader reads. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, image->GetFullRange());

		/* Update the descriptor. */
		material = new DescriptorSet(std::move(pipeline->GetDescriptorPool().Allocate(1)));
		material->Write(pipeline->GetRenderpass().GetUniform("Albedo"), *image);
	}
	else	// Render ImGui
	{
		ImGui::Begin("Performance");
		ImGui::Text("FPS: %d", iround(1.0f / dt));

		vector<uint32> timestamps = queryPool->GetResults(0, 2, true, false);
		if (timestamps[0])
		{
			const float period = GetDevice().GetPhysicalDevice().GetLimits().TimestampPeriod;
			ImGui::SameLine();
			ImGui::Text("(%f ms)", (timestamps[1] - timestamps[0]) * period * 0.000001f);
		}

		ImGui::Text("CPU: %d%%", ipart(CPU::GetCurrentProcessUsage() * 100.0f));

		ImGui::End();
	}

	/* Timestamps. */
	cmdBuffer.WriteTimestamp(PipelineStageFlag::TopOfPipe, *queryPool, 0);
	cmdBuffer.WriteTimestamp(PipelineStageFlag::BottomOfPipe, *queryPool, 1);

	/* Render scene. */
	cmdBuffer.AddLabel(u8"Monster", Color::Lime());
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass()), SubpassContents::Inline);

	cmdBuffer.BindIndexBuffer(*index, IndexType::UInt16);
	cmdBuffer.BindVertexBuffer(0, *mesh);
	cmdBuffer.BindGraphicsDescriptor(*transform);
	cmdBuffer.BindGraphicsDescriptor(*material);
	cmdBuffer.Draw(static_cast<uint32>(index->GetElementCount()), 1, 0, 0, 0);

	cmdBuffer.EndRenderPass();
	cmdBuffer.EndLabel();
}