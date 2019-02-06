#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Graphics/VertexLayouts/Image2D.h>
#include <Graphics/Resources/ImageHandler.h>
#include <Core/Math/Matrix.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application("TestGame")
{}

void TestGame::Initialize(void)
{
	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [this](GraphicsPipeline &pipeline, EventArgs)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleStrip);
		pipeline.AddVertexBinding<Image2D>(0);
		pipeline.Finalize();

		/* Allocate and move the new descriptor set. */
		descriptor = new DescriptorSet(std::move(pipeline.GetDescriptorPool().Allocate(0)));

		/* Create the framebuffers with no extra image views. */
		GetWindow().CreateFrameBuffers(pipeline.GetRenderpass(), {});
		TempMarkDoneLoading();
	};

	/* Setup and load render pass. */
	GetContent().FetchRenderpass(*pipeline, { "../assets/shaders/Image.vert", "../assets/shaders/Image.frag" }).OnLinkCompleted += [this](Renderpass &renderpass, EventArgs)
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

	const Matrix identity = Matrix::CreateScalar(2.0f);

	/* Initialize the final vertex buffer and setup the staging buffer with our quad. */
	vrtxBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst);
	vrtxStagingBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::TransferSrc, true);
	vrtxStagingBuffer->SetData(quad, 4);

	/* Load image from disk. */
	const ImageInformation imgInfo = _CrtGetImageInfo("../assets/images/Plutonium.png");
	const vector<byte> texels = _CrtLoadImageLDR("../assets/images/Plutonium.png");

	/* Initializes our image and view. */
	sampler = new Sampler(GetDevice(), SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat));
	image = new Texture2D(GetDevice(), *sampler, Format::R8G8B8A8_SRGB, Extent2D(imgInfo.Width, imgInfo.Height), ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled);

	/* Copy texel information to staging buffer. */
	imgStagingBuffer = new Buffer(GetDevice(), texels.size() * sizeof(float), BufferUsageFlag::TransferSrc, true);
	imgStagingBuffer->SetData(texels.data(), texels.size());

	/* Initialize the uniform buffer and setup the staging buffer. */
	uniBuffer = new Buffer(GetDevice(), sizeof(identity), BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst);
	uniStagingBuffer = new Buffer(GetDevice(), sizeof(identity), BufferUsageFlag::TransferSrc, true);
	uniStagingBuffer->SetData(identity.GetComponents(), 16);
}

void TestGame::UnLoadContent(void)
{
	delete uniStagingBuffer;
	delete uniBuffer;

	delete imgStagingBuffer;
	delete image;
	delete sampler;

	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	GetContent().Release(*pipeline);

	delete descriptor;
	delete pipeline;
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
		cmdBuffer.CopyEntireBuffer(*imgStagingBuffer, *image);
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, DependencyFlag::None, ImageLayout::ShaderReadOnlyOptimal,
			AccessFlag::ShaderRead, QueueFamilyIgnored, ImageSubresourceRange());

		/* Copy matrix to final uniform buffer. */
		cmdBuffer.CopyEntireBuffer(*uniStagingBuffer, *uniBuffer);
		cmdBuffer.MemoryBarrier(*uniBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexShader, DependencyFlag::None, AccessFlag::UniformRead);

		/* Update the descriptor. */
		descriptor->Write(pipeline->GetRenderpass().GetUniform("Texture"), *image);
		/* Update projection matrix. */
		descriptor->Write(pipeline->GetRenderpass().GetUniform("Projection"), *uniBuffer);
	}

	/* Get the current render area and get our current framebuffer. */
	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass());

	/* Render scene. */
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), framebuffer, renderArea, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);

	cmdBuffer.BindVertexBuffer(0, *vrtxBuffer);
	cmdBuffer.BindGraphicsDescriptor(*descriptor);

	cmdBuffer.Draw(vrtxBuffer->GetElementCount(), 1, 0, 0);

	cmdBuffer.EndRenderPass();
}