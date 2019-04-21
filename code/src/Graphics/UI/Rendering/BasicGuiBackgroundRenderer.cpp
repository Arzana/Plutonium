#include "Graphics/UI/Rendering/BasicGuiBackgroundRenderer.h"
#include "Graphics/VertexLayouts/Image2D.h"

Pu::BasicGuiBackgroundRenderer::BasicGuiBackgroundRenderer(GameWindow & window, AssetFetcher & loader, size_t maxSets)
	: Renderer(window, loader, maxSets, { L"{Shaders}2D.vert", L"{Shaders}UIBackground.frag" })
{}

Pu::GuiBackgroundUniformBlock* Pu::BasicGuiBackgroundRenderer::CreateGUI(void) const
{
	return new GuiBackgroundUniformBlock(GetWindow().GetDevice(), GetPipeline());
}

void Pu::BasicGuiBackgroundRenderer::Render(const BufferView & mesh, const GuiBackgroundUniformBlock & uniforms)
{
	CmdBuffer->BindVertexBuffer(0, mesh);
	CmdBuffer->BindGraphicsDescriptor(uniforms);
	CmdBuffer->Draw(static_cast<uint32>(mesh.GetElementCount()), 1, 0, 0);
}

void Pu::BasicGuiBackgroundRenderer::OnPipelinePostInitialize(GraphicsPipeline & gfx)
{
	/* Set the required parameters. */
	gfx.SetViewport(GetWindow().GetNative().GetClientBounds());
	gfx.SetTopology(PrimitiveTopology::TriangleList);
	gfx.AddVertexBinding<Image2D>(0);
	gfx.Finalize();

	/* Create the initial framebuffers. */
	GetWindow().CreateFrameBuffers(gfx.GetRenderpass());
}

void Pu::BasicGuiBackgroundRenderer::OnRenderpassLinkComplete(Renderpass & renderpass)
{
	Output &output = renderpass.GetOutput("FragColor");
	output.SetDescription(GetWindow().GetSwapchain());
	output.SetLoadOperation(AttachmentLoadOp::Load);
	output.SetColorBlending(BlendFactor::SrcAlpha, BlendOp::Add, BlendFactor::ISrcAlpha);
	output.SetAlphaBlending(BlendFactor::One, BlendOp::Add, BlendFactor::Zero);

	renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Image2D, TexCoord));
}

void Pu::BasicGuiBackgroundRenderer::RecreateFramebuffers(GameWindow & window, const Renderpass & renderpass)
{
	window.CreateFrameBuffers(renderpass);
}