#include "Graphics/Text/TextRenderer.h"
#include "Graphics/VertexLayouts/Image2D.h"

Pu::TextRenderer::TextRenderer(GameWindow & window, AssetFetcher & loader, size_t maxSets)
	: Renderer(window, loader, maxSets, { L"{Shaders}2D.vert.spv", L"{Shaders}Text.frag.spv" })
{}

Pu::TextUniformBlock * Pu::TextRenderer::CreateText(void) const
{
	return new TextUniformBlock(GetPipeline());
}

Pu::DescriptorSet * Pu::TextRenderer::CreatFont(_In_ const Texture2D & atlas) const
{
	DescriptorSet *result = new DescriptorSet(std::move(GetPipeline().GetDescriptorPool().Allocate(1)));
	result->Write(GetPipeline().GetRenderpass().GetUniform("Atlas"), atlas);
	return result;
}

void Pu::TextRenderer::Begin(CommandBuffer &cmdBuffer)
{
	if (CanBegin()) cmdBuffer.AddLabel(u8"TextRenderer", Color::Cyan());
	Renderer::Begin(cmdBuffer);
}

void Pu::TextRenderer::SetFont(const DescriptorSet & info)
{
#ifdef _DEBUG
	if (!CmdBuffer) Log::Fatal("Attempting to set font on text renderer without calling Begin first!");
#endif

	CmdBuffer->BindGraphicsDescriptor(info);
}

void Pu::TextRenderer::Render(const TextBuffer & text, const TextUniformBlock & uniforms)
{
#ifdef _DEBUG
	if (!CmdBuffer) Log::Fatal("Attempting to render text on text renderer without calling Begin first!");
#endif

	/* Render the string. */
	CmdBuffer->BindVertexBuffer(0, text.GetView());
	CmdBuffer->BindGraphicsDescriptor(uniforms);
	CmdBuffer->Draw(static_cast<uint32>(text.GetView().GetElementCount()), 1, 0, 0);
}

void Pu::TextRenderer::End(void)
{
	CommandBuffer *cmdBuffer = CmdBuffer;
	Renderer::End();

	if (CanBegin()) cmdBuffer->EndLabel();
}

void Pu::TextRenderer::OnPipelinePostInitialize(GraphicsPipeline &gfx)
{
	/* Set the required parameters. */
	gfx.SetViewport(GetWindow().GetNative().GetClientBounds());
	gfx.SetTopology(PrimitiveTopology::TriangleList);
	gfx.AddVertexBinding<Image2D>(0);
	gfx.Finalize(0);

	/* Create the framebuffers for this graphics pipeline. */
	GetWindow().CreateFrameBuffers(gfx.GetRenderpass());
}

void Pu::TextRenderer::OnRenderpassLinkComplete(Renderpass & renderpass)
{
	Output &output = renderpass.GetOutput("FragColor");
	output.SetDescription(GetWindow().GetSwapchain());
	output.SetLoadOperation(AttachmentLoadOp::Load);
	output.SetColorBlending(BlendFactor::SrcAlpha, BlendOp::Add, BlendFactor::ISrcAlpha);
	output.SetAlphaBlending(BlendFactor::One, BlendOp::Add, BlendFactor::Zero);

	/* Set the required parameters. */
	renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Image2D, TexCoord));
}

void Pu::TextRenderer::RecreateFramebuffers(GameWindow & window, const Renderpass & renderpass)
{
	window.CreateFrameBuffers(renderpass);
}