#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Graphics/VertexLayouts/ColoredVertex2D.h>

using namespace Pu;

GraphicsPipeline::LoadTask *loader;

TestGame::TestGame(void)
	: Application("TestGame"), renderpass(nullptr)
{}

void TestGame::Initialize(void)
{
	renderpass = new Renderpass(GetDevice());
	renderpass->OnLinkCompleted += [&](Renderpass &renderpass, EventArgs)
	{
		Output &fragColor = renderpass.GetOutput("FragColor");
		fragColor.SetDescription(GetWindow().GetSwapchain());
		fragColor.SetLayout(ImageLayout::ColorAttachmentOptimal);

		Attribute &clr = renderpass.GetAttribute("Color");
		clr.SetOffset(vkoffsetof(ColoredVertex2D, Color));

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

	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [&](GraphicsPipeline &pipeline, EventArgs)
	{
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleStrip);
		pipeline.AddVertexBinding<ColoredVertex2D>(0);
		pipeline.Finalize();

		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(*renderpass, views);

		TempMarkDoneLoading();
	};

	loader = new GraphicsPipeline::LoadTask(*pipeline, *renderpass, { "../assets/shaders/VertexColor2D.vert", "../assets/shaders/VertexColor.frag" });
	ProcessTask(*loader);

	GetWindow().GetNative().OnSizeChanged += [&](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(*renderpass, views);
	};
}

void TestGame::LoadContent(void)
{
	const ColoredVertex2D quad[] =
	{
		{ Vector2(-0.7f, -0.7f), Color::Red() },
		{ Vector2(-0.7f, 0.7f), Color::Green() },
		{ Vector2(0.7f, -0.7f), Color::Blue() },
		{ Vector2(0.7f, 0.7f), Color::Tundora() }
	};

	vrtxBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst);
	stagingBuffer = new Buffer(GetDevice(), sizeof(quad), BufferUsageFlag::TransferSrc, true);
	stagingBuffer->SetData(quad, 4);
}

void TestGame::UnLoadContent(void)
{
	delete stagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
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
		cmdBuffer.CopyEntireBuffer(*stagingBuffer, *vrtxBuffer);

		BufferMemoryBarrier barrier(vrtxBuffer->GetHandle());
		barrier.SrcAccessMask = AccessFlag::memoryWrite;
		barrier.DstAccessMask = AccessFlag::VertexAttributeRead;

		cmdBuffer.BufferMemoryBarrier(PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, DependencyFlag::None, { barrier });
	}

	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(*renderpass);

	cmdBuffer.BeginRenderPass(*renderpass, framebuffer, renderArea, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BindVertexBuffer(0, *vrtxBuffer);
	cmdBuffer.Draw(vrtxBuffer->GetElementCount(), 1, 0, 0);
	cmdBuffer.EndRenderPass();
}