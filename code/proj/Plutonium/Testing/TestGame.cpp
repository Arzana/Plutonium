#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Graphics/VertexLayouts/Image2D.h>
#include <Graphics/Resources/ImageHandler.h>

using namespace Pu;

GraphicsPipeline::LoadTask *loader;

TestGame::TestGame(void)
	: Application("TestGame"), renderpass(nullptr)
{}

void TestGame::Initialize(void)
{
	/* Setup render pass. */
	renderpass = new Renderpass(GetDevice());
	renderpass->OnLinkCompleted += [&](Renderpass &renderpass, EventArgs)
	{
		/* Set description and layout of FragColor. */
		Output &fragColor = renderpass.GetOutput("FragColor");
		fragColor.SetDescription(GetWindow().GetSwapchain());
		fragColor.SetLayout(ImageLayout::ColorAttachmentOptimal);

		/* Set offset for uv attribute (position is default). */
		Attribute &uv = renderpass.GetAttribute("TexCoord");
		uv.SetOffset(vkoffsetof(Image2D, TexCoord));

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

	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [&](GraphicsPipeline &pipeline, EventArgs)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleStrip);
		pipeline.AddVertexBinding<Image2D>(0);
		pipeline.Finalize();

		/* Allocate and move the new descriptor set. */
		descriptor = new DescriptorSet(std::move(pipeline.GetDescriptorPool().Allocate(0)));

		/* Create the framebuffers with no extra image views. */
		GetWindow().CreateFrameBuffers(*renderpass, {});
		TempMarkDoneLoading();
	};

	/* Start loading the graphics pipeline. */
	loader = new GraphicsPipeline::LoadTask(*pipeline, *renderpass, { "../assets/shaders/Image.vert", "../assets/shaders/Image.frag" });
	ProcessTask(*loader);

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().GetNative().OnSizeChanged += [&](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(*renderpass, views);
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
	vrtxBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst);
	vrtxStagingBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::TransferSrc, true);
	vrtxStagingBuffer->SetData(quad, 4);

	/* Load image from disk. */
	ImageInformation imgInfo;
	const vector<float> texels = _CrtLoadImage("../assets/images/Plutonium.png", imgInfo, 4);

	/* Initializes our image and view. */
	image = new Image(GetDevice(), ImageCreateInfo(ImageType::Image2D, Format::R32G32B32A32_SFLOAT, Extent3D(imgInfo.Width, imgInfo.Height, 1), ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled));
	view = new ImageView(GetDevice(), *image);

	/* Copy texel information to staging buffer. */
	imgStagingBuffer = new Buffer(GetDevice(), texels.size() * sizeof(float), BufferUsageFlag::TransferSrc, true);
	imgStagingBuffer->SetData(texels.data(), texels.size());
}

void TestGame::UnLoadContent(void)
{
	delete imgStagingBuffer;
	delete view;
	delete image;

	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	delete descriptor;
	delete pipeline;
	delete renderpass;
	delete loader;
}

void TestGame::Render(float, CommandBuffer & cmdBuffer)
{
	static bool firstRender = true;
	if (firstRender)
	{
		firstRender = false;

		/* Copy quad to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, DependencyFlag::None, AccessFlag::VertexAttributeRead);

		/* Copy image to final image. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, DependencyFlag::None, ImageLayout::TransferDstOptimal,
			AccessFlag::TransferWrite, QueueFamilyIgnored, ImageSubresourceRange());
		cmdBuffer.CopyBuffer(*imgStagingBuffer, *image, { BufferImageCopy(image->GetSize()) });
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, DependencyFlag::None, ImageLayout::ShaderReadOnlyOptimal,
			AccessFlag::ShaderRead, QueueFamilyIgnored, ImageSubresourceRange());

		/* Update the descriptor. */
		descriptor->Write(renderpass->GetUniform("Texture"), *view);
	}

	/* Get the current render area and get our current framebuffer. */
	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(*renderpass);

	/* Render scene. */
	cmdBuffer.BeginRenderPass(*renderpass, framebuffer, renderArea, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);

	cmdBuffer.BindVertexBuffer(0, *vrtxBuffer);
	cmdBuffer.BindGraphicsDescriptor(*descriptor);

	cmdBuffer.Draw(vrtxBuffer->GetElementCount(), 1, 0, 0);

	cmdBuffer.EndRenderPass();
}