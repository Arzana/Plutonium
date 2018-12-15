#include "TestGame.h"

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
}

void TestGame::LoadContent(void)
{
	TempMarkDoneLoading();
}

void TestGame::Finalize(void)
{
	delete renderpass;
}

void TestGame::Render(float)
{
	GetWindow().GetCommandBuffer().ClearImage(GetWindow().GetCurrentImage(), Color::Orange());
}