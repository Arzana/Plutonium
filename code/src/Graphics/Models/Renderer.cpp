#include "Graphics/Models/Renderer.h"

Pu::Renderer::Renderer(GameWindow & window, AssetFetcher & loader, size_t maxSets, std::initializer_list<wstring> shaders)
	: wnd(window), loader(loader), CmdBuffer(nullptr)
{
	/* Create the graphics pipeline. */
	pipeline = new GraphicsPipeline(window.GetDevice(), maxSets);
	pipeline->PostInitialize.Add(*this, &Renderer::OnPipelinePostInitialize);

	/* Create the renderpass. */
	loader.FetchRenderpass(*pipeline, shaders).OnLinkCompleted.Add(*this, &Renderer::OnRenderpassLinkComplete);
	window.SwapchainRecreated.Add(*this, &Renderer::OnSwapchainChanged);
}

Pu::Renderer::~Renderer(void)
{
	loader.Release(*pipeline);
	delete pipeline;
}

bool Pu::Renderer::CanBegin(void)
{
	return !CmdBuffer && pipeline->IsUsable();
}

void Pu::Renderer::Begin(CommandBuffer & cmdBuffer)
{
	if (CanBegin())
	{
		/* Starts the pipeline. */
		CmdBuffer = &cmdBuffer;
		cmdBuffer.BindGraphicsPipeline(*pipeline);

		/* Begin the renderpass. */
		const Renderpass &renderpass = pipeline->GetRenderpass();
		cmdBuffer.BeginRenderPass(renderpass, wnd.GetCurrentFramebuffer(renderpass), SubpassContents::Inline);
	}
	else Log::Warning("Attempting to start invalid renderer!");
}

void Pu::Renderer::End(void)
{
	if (CanBegin()) Log::Warning("Attempting to end invalid renderer!");
	else
	{
		CmdBuffer->EndRenderPass();
		CmdBuffer = nullptr;
		CmdBuffer = nullptr;
	}
}

void Pu::Renderer::OnSwapchainChanged(const GameWindow&)
{
	if (pipeline->IsUsable()) RecreateFramebuffers(wnd, pipeline->GetRenderpass());
}