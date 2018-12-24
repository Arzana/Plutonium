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
		renderpass.GetOutput("FragColor").SetDescription(GetWindow().GetSwapchain());
	};

	pipeline = new GraphicsPipeline(GetDevice());
	pipeline->PostInitialize += [&](GraphicsPipeline &pipeline, EventArgs)
	{
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.Finalize();

		TempMarkDoneLoading();
	};

	loader = new GraphicsPipeline::LoadTask(*pipeline, *renderpass, { "../assets/shaders/Triangle.vert", "../assets/shaders/Triangle.frag" });
	ProcessTask(*loader);
}

void TestGame::LoadContent(void)
{}

void TestGame::Finalize(void)
{
	delete pipeline;
	delete renderpass;
	delete loader;
}

void TestGame::Render(float, CommandBuffer & cmdBuffer)
{
	vector<const ImageView*> views;
	views.push_back(&GetWindow().GetCurrentImageView());
	const Extent2D dimensions = GetWindow().GetNative().GetClientBounds().GetSize();
	Framebuffer framebuffer(GetDevice(), *renderpass, dimensions, views);

	cmdBuffer.BeginRenderPass(*renderpass, framebuffer, Rect2D(dimensions), SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.Draw(3, 1, 0, 0);
	cmdBuffer.EndRenderPass();
}