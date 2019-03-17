#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Core/Math/Matrix.h>
#include <Input/Keys.h>
#include <Content/GLTFParser.h>

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
		pipeline.AddVertexBinding<SkinnedAnimated>(0);
		pipeline.Finalize();

		/* Allocate and move the new descriptor set. */
		descriptor = new DescriptorSet(std::move(pipeline.GetDescriptorPool().Allocate(0)));

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
		Attribute &uv = renderpass.GetAttribute("TexCoord");
		uv.SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));

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
	GLTFFile result;
	_CrtLoadGLTF(L"../assets/models/Monster/Monster.gltf", result);

	const Matrix identity = Matrix::CreateScalar(0.2f);

	/* Initialize the final vertex buffer and setup the staging buffer with our quad. */
	vrtxBuffer = new Buffer(GetDevice(), result.Buffers[0].Size, BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst, false);

	vector<std::reference_wrapper<Buffer>> buffers;
	buffers.emplace_back(*vrtxBuffer);
	vector<StagingBuffer*> stagingBuffers;
	vector<Mesh*> meshes;
	_CrtLoadAndParseGLTF(result, buffers, stagingBuffers, meshes);
	vrtxStagingBuffer = stagingBuffers[0];
	mesh = meshes[0];

	/* Initialize the uniform buffer and setup the staging buffer. */
	uniBuffer = new Buffer(GetDevice(), sizeof(identity), BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst, false);
	uniStagingBuffer = new StagingBuffer(*uniBuffer);
	uniStagingBuffer->Load(identity.GetComponents());

	/* Load the texture. */
	image = &GetContent().FetchTexture2D(result.Images[0].Uri, SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat));
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);

	delete uniStagingBuffer;
	delete uniBuffer;

	delete mesh;
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
		/* Wait for the graphics pipeline to be usable. */
		if (descriptor == nullptr || !image->IsUsable()) return;
		firstRender = false;

		/* Copy quad to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		/* Make sure the image layout is suitable for shader reads. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, image->GetFullRange());

		/* Copy matrix to final uniform buffer. */
		cmdBuffer.CopyEntireBuffer(*uniStagingBuffer, *uniBuffer);
		cmdBuffer.MemoryBarrier(*uniBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexShader, AccessFlag::UniformRead);

		/* Update the descriptor. */
		descriptor->Write(pipeline->GetRenderpass().GetUniform("Texture"), *image);
		/* Update projection matrix. */
		descriptor->Write(pipeline->GetRenderpass().GetUniform("Projection"), *uniBuffer);
	}

	/* Get the current render area and get our current framebuffer. */
	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass());

	/* Render scene. */
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), framebuffer, renderArea, SubpassContents::Inline);

	cmdBuffer.BindVertexBuffer(0, *mesh);
	cmdBuffer.BindGraphicsDescriptor(*descriptor);

	cmdBuffer.Draw(mesh->GetElementCount(), 1, 0, 0);

	cmdBuffer.EndRenderPass();
}