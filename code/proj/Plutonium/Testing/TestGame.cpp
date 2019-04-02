#include "TestGame.h"
#include <Input/Keys.h>
#include <Content/GLTFParser.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>

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

	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [this](GraphicsPipeline &pipeline)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleList);
		pipeline.AddVertexBinding(0, sizeof(Vector3) * 2);
		pipeline.Finalize();

		/* Create the framebuffers and the required uniform block. */
		GetWindow().CreateFrameBuffers(pipeline.GetRenderpass());
		transform = new TransformBlock(GetDevice(), pipeline);
	};

	/* Setup and load render pass. */
	GetContent().FetchRenderpass(*pipeline, { L"{Shaders}TestBox.vert", L"{Shaders}TestBox.frag" }).OnLinkCompleted += [this](Renderpass &renderpass)
	{
		/* Set description and layout of FragColor. */
		Output &fragColor = renderpass.GetOutput("FragColor");
		fragColor.SetDescription(GetWindow().GetSwapchain());
		fragColor.SetLayout(ImageLayout::ColorAttachmentOptimal);

		/* Set offset for uv attribute (position is default). */
		renderpass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
	};

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().GetNative().OnSizeChanged += [this](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(pipeline->GetRenderpass(), views);
	};
}

void TestGame::LoadContent(void)
{
	GLTFFile file;
	_CrtLoadGLTF(L"../assets/models/Testing/Box/Box.gltf", file);

	/* Initialize the final vertex buffer and setup the staging buffer with our quad. */
	vrtxBuffer = new Buffer(GetDevice(), file.Buffers[0].Size, BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer | BufferUsageFlag::TransferDst, false);

	vector<std::reference_wrapper<Buffer>> buffers;
	buffers.emplace_back(*vrtxBuffer);
	vector<StagingBuffer*> stagingBuffers;
	vector<Mesh*> meshes;

	_CrtLoadAndParseGLTF(file, buffers, stagingBuffers, meshes);
	vrtxStagingBuffer = stagingBuffers[0];
	mesh = meshes[0];

	/* Load the texture. */
	image = &GetContent().FetchTexture2D(L"{Textures}Uv.png", SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat));
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);

	delete mesh;
	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	GetContent().Release(*pipeline);

	delete transform;
	delete pipeline;
}

void TestGame::Update(float)
{
	if (transform)
	{
		transform->SetProjection(cam->GetProjection());
		transform->SetView(cam->GetView());
	}
}

void TestGame::Render(float, CommandBuffer & cmdBuffer)
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

		/* Copy quad to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		/* Make sure the image layout is suitable for shader reads. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, image->GetFullRange());

		/* Update the descriptor. */
		//transform->SetTexture(pipeline->GetRenderpass().GetUniform("Texture"), *image);
	}

	/* Render scene. */
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass()), SubpassContents::Inline);

	cmdBuffer.BindVertexBuffer(0, *mesh);
	cmdBuffer.BindIndexBuffer(mesh->GetIndex());
	cmdBuffer.BindGraphicsDescriptor(const_cast<const TransformBlock*>(transform)->GetDescriptor());
	cmdBuffer.Draw(mesh->GetIndex().GetElementCount(), 1, 0, 0, 0);

	cmdBuffer.EndRenderPass();
}