#include "TestGame.h"
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>

using namespace Pu;

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

	Renderpass::LoadTask loader(*renderpass, { "../assets/shaders/Triangle.vert", "../assets/shaders/Triangle.frag" });
	ProcessTask(loader);

	while (!renderpass->IsLoaded()) {}

	pipeline = new GraphicsPipeline(GetDevice(), *renderpass);
	pipeline->SetViewport(GetWindow().GetNative().GetClientBounds());
	pipeline->Finalize();

	GetWindow().GetNative().OnSizeChanged += [&](const NativeWindow &wnd, ValueChangedEventArgs<Vector2>)
	{
		pipeline->SetViewport(wnd.GetClientBounds());
		pipeline->Finalize();
	};
}

void TestGame::LoadContent(void)
{
	TempMarkDoneLoading();
}

void TestGame::Finalize(void)
{
	delete pipeline;
	delete renderpass;
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