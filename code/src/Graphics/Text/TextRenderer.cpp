#include "Graphics/Text/TextRenderer.h"
#include "Graphics/VertexLayouts/Image2D.h"

Pu::TextRenderer::TextRenderer(GameWindow & window, AssetFetcher & loader, std::initializer_list<wstring> shaders)
	: wnd(window), loader(loader), curCmdBuffer(nullptr)
{
	/* Create the gaphics pipeline. */
	pipeline = new GraphicsPipeline(window.GetDevice());
	pipeline->PostInitialize.Add(*this, &TextRenderer::OnPipelinePostInitialize);

	/* Create the renderpass. */
	loader.FetchRenderpass(*pipeline, shaders).OnLinkCompleted.Add(*this, &TextRenderer::OnRenderpassLinkComplete);
	wnd.GetNative().OnSizeChanged.Add(*this, &TextRenderer::OnWindowSizeChanged);
}

Pu::TextRenderer::~TextRenderer(void)
{
	loader.Release(*pipeline);
	delete pipeline;
}

bool Pu::TextRenderer::CanBegin(void) const
{
	return !curCmdBuffer && pipeline->IsUsable();
}

Pu::TextUniformBlock Pu::TextRenderer::CreateUniformBlock(void) const
{
	return TextUniformBlock(wnd.GetDevice(), *pipeline);
}

void Pu::TextRenderer::Begin(CommandBuffer &cmdBuffer)
{
	/* Make sure we can actually begin this renderpass. */
	if (CanBegin())
	{
		/* Set the command buffer and start the pipeline. */
		curCmdBuffer = &cmdBuffer;
		cmdBuffer.BindGraphicsPipeline(*pipeline);

		/* Begin the renderpass. */
		const Renderpass &renderpass = pipeline->GetRenderpass();
		cmdBuffer.BeginRenderPass(renderpass, wnd.GetCurrentFramebuffer(renderpass), SubpassContents::Inline);
	}
	else Log::Warning("Attempting to start invalid text renderer!");
}

void Pu::TextRenderer::Render(const TextBuffer & text, const TextUniformBlock & uniforms)
{
#ifdef _DEBUG
	if (!curCmdBuffer) Log::Fatal("Attempting to render text on text renderer without calling Begin first!");
#endif

	/* Render the string. */
	curCmdBuffer->BindVertexBuffer(0, text.GetView());
	curCmdBuffer->BindGraphicsDescriptor(uniforms.GetDescriptor());
	curCmdBuffer->Draw(static_cast<uint32>(text.GetView().GetElementCount()), 1, 0, 0);
}

void Pu::TextRenderer::End(void)
{
	if (CanBegin()) Log::Warning("Attempting to stop invalid text renderer!");
	else
	{
		curCmdBuffer->EndRenderPass();
		curCmdBuffer = nullptr;
	}
}

void Pu::TextRenderer::OnPipelinePostInitialize(GraphicsPipeline &)
{
	/* Set the required parameters. */
	pipeline->SetViewport(wnd.GetNative().GetClientBounds());
	pipeline->SetTopology(PrimitiveTopology::TriangleList);
	pipeline->SetDepthStencilMode(false, false, false);
	pipeline->AddVertexBinding(0, sizeof(Image2D));
	pipeline->SetCullMode(CullModeFlag::None);
	pipeline->Finalize();

	/* Create the framebuffers for this graphics pipeline. */
	wnd.CreateFrameBuffers(pipeline->GetRenderpass());
}

void Pu::TextRenderer::OnRenderpassLinkComplete(Renderpass & renderpass)
{
	Output &output = renderpass.GetOutput("FragColor");
	output.SetDescription(wnd.GetSwapchain());
	output.SetLoadOperation(AttachmentLoadOp::Load);

	/* Set the required parameters. */
	renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Image2D, TexCoord));
}

void Pu::TextRenderer::OnWindowSizeChanged(const NativeWindow &, ValueChangedEventArgs<Vector2>)
{
	if (CanBegin()) wnd.CreateFrameBuffers(pipeline->GetRenderpass());
}