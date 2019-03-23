#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Graphics/VertexLayouts/Image2D.h>
#include <Core/Math/Matrix.h>
#include <Input/Keys.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 1920.0f, 1080.0f)
{
	GetInput().AnyKeyboard.KeyDown += [this](const Keyboard&, uint16 key)
	{
		if (key == _CrtEnum2Int(Keys::Escape)) Exit();
	};
}

void TestGame::Initialize(void)
{
	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [this](GraphicsPipeline &pipeline, EventArgs)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleStrip);
		pipeline.AddVertexBinding(0, sizeof(Image2D));
		pipeline.Finalize();

		/* Create the transform uniform block. */
		transform = new TransformBlock(GetDevice(), pipeline.GetDescriptorPool(), 0, pipeline.GetRenderpass().GetUniform("Projection"));

		/* Create the framebuffers with no extra image views. */
		GetWindow().CreateFrameBuffers(pipeline.GetRenderpass(), {});
	};

	/* Setup and load render pass. */
	GetContent().FetchRenderpass(*pipeline, { L"../assets/shaders/Image.vert", L"../assets/shaders/Image.frag" }).OnLinkCompleted += [this](Renderpass &renderpass, EventArgs)
	{
		/* Set description and layout of FragColor. */
		Output &fragColor = renderpass.GetOutput("FragColor");
		fragColor.SetDescription(GetWindow().GetSwapchain());
		fragColor.SetLayout(ImageLayout::ColorAttachmentOptimal);

		/* Set offset for uv attribute (position is default). */
		renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Image2D, TexCoord));

		/* Setup the subpass dependencies. */
		SubpassDependency dependency(SubpassExternal, 0);
		dependency.SrcStageMask = PipelineStageFlag::BottomOfPipe;
		dependency.DstStageMask = PipelineStageFlag::ColorAttachmentOutput;
		dependency.SrcAccessMask = AccessFlag::MemoryRead;
		dependency.DstAcccessMask = AccessFlag::ColorAttachmentWrite;
		dependency.DependencyFlags = DependencyFlag::ByRegion;
		renderpass.AddDependency(dependency);

		dependency = SubpassDependency(0, SubpassExternal);
		dependency.SrcStageMask = PipelineStageFlag::ColorAttachmentOutput;
		dependency.DstStageMask = PipelineStageFlag::BottomOfPipe;
		dependency.SrcAccessMask = AccessFlag::ColorAttachmentWrite;
		dependency.DstAcccessMask = AccessFlag::MemoryRead;
		dependency.DependencyFlags = DependencyFlag::ByRegion;
		renderpass.AddDependency(dependency);
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
	const Image2D quad[] =
	{
		{ Vector2(-0.7f, -0.7f), Vector2(0.0f, 0.0f) },
		{ Vector2(-0.7f, 0.7f), Vector2(0.0f, 1.0f) },
		{ Vector2(0.7f, -0.7f), Vector2(1.0f, 0.0f) },
		{ Vector2(0.7f, 0.7f), Vector2(1.0f, 1.0f) }
	};

	/* Initialize the final vertex buffer and setup the staging buffer with our quad. */
	vrtxBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst, false);
	vrtxStagingBuffer = new StagingBuffer(*vrtxBuffer);
	vrtxStagingBuffer->Load(quad);

	/* Load the texture. */
	image = &GetContent().FetchTexture2D(L"../assets/images/uv.png", SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat));
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);

	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	GetContent().Release(*pipeline);

	delete transform;
	delete pipeline;
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
		transform->SetTexture(pipeline->GetRenderpass().GetUniform("Texture"), *image);
	}

	/* Get the current render area and get our current framebuffer. */
	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass());

	/* Render scene. */
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), framebuffer, renderArea, SubpassContents::Inline);

	cmdBuffer.BindVertexBuffer(0, BufferView(*vrtxBuffer, sizeof(Image2D)));
	cmdBuffer.BindGraphicsDescriptor(const_cast<const TransformBlock*>(transform)->GetDescriptor());
	cmdBuffer.Draw(4, 1, 0, 0);

	cmdBuffer.EndRenderPass();
}