#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>

using namespace Pu;

GraphicsPipeline::LoadTask *loader;

TestGame::TestGame(void)
	: Application("TestGame"), renderpass(nullptr)
{}

bool TestGame::GpuPredicate(const PhysicalDevice & physicalDevice)
{
	return physicalDevice.GetType() == PhysicalDeviceType::DiscreteGpu;
}

void TestGame::Initialize(void)
{
	renderpass = new Renderpass(GetDevice());
	renderpass->OnAttachmentLink += [&](Renderpass &renderpass, EventArgs)
	{
		Output &fragColor = renderpass.GetOutput("FragColor");
		fragColor.SetDescription(GetWindow().GetSwapchain());
		fragColor.SetLayout(ImageLayout::ColorAttachmentOptimal);

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
		pipeline.Finalize();

		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(*renderpass, views);

		TempMarkDoneLoading();
	};

	loader = new GraphicsPipeline::LoadTask(*pipeline, *renderpass, { "../assets/shaders/Triangle.vert", "../assets/shaders/VertexColor.frag" });
	ProcessTask(*loader);

	GetWindow().GetNative().OnSizeChanged += [&](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		const vector<const ImageView*> views;
		GetWindow().CreateFrameBuffers(*renderpass, views);
	};
}

void TestGame::Finalize(void)
{
	delete pipeline;
	delete renderpass;
	delete loader;
}

void TestGame::Render(float, CommandBuffer & cmdBuffer)
{
	const Rect2D renderArea = GetWindow().GetNative().GetClientBounds().GetSize();
	const Framebuffer &framebuffer = GetWindow().GetCurrentFramebuffer(*renderpass);

	cmdBuffer.BeginRenderPass(*renderpass, framebuffer, renderArea, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.Draw(3, 1, 0, 0);
	cmdBuffer.EndRenderPass();
}