#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Graphics/Diagnostics/DebugRendererUniformBlock.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DebugRenderer::DebugRenderer(GameWindow & window, AssetFetcher & loader, const DepthBuffer * depthBuffer, float lineWidth)
	: Renderer(window, loader, 1, { L"{Shaders}VertexColor.vert.spv", L"{Shaders}VertexColor.frag.spv" }),
	uniforms(nullptr), size(0), lineWidth(lineWidth), depthBuffer(depthBuffer)
{
	/* We need a dynamic buffer because the data will update every frame, the queue is a raw array to increase performance. */
	buffer = new DynamicBuffer(window.GetDevice(), sizeof(ColoredVertex3D) * MaxDebugRendererVertices, BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer);
	bufferView = new BufferView(*buffer, sizeof(ColoredVertex3D));
	queue = reinterpret_cast<ColoredVertex3D*>(malloc(sizeof(ColoredVertex3D) * MaxDebugRendererVertices));

	/*
	Static line width is a feature that not all graphics cards support
	so we can either set it in the graphics pipeline (fastest)
	or we ned to add it as a command (works on all devices).
	*/
	dynamicLineWidth = !window.GetDevice().GetPhysicalDevice().GetFeatures().WideLines;
}

Pu::DebugRenderer::~DebugRenderer(void)
{
	delete buffer;
	delete bufferView;
	delete queue;
	if (uniforms) delete uniforms;
}

void Pu::DebugRenderer::AddLine(const Line & line, Color color)
{
	AddLine(line.Start, line.End, color);
}

void Pu::DebugRenderer::AddLine(Vector3 start, Vector3 end, Color color)
{
	/* Only add the line if we have enough space. */
	if (size + 2 <= MaxDebugRendererVertices)
	{
		AddVertex(start, color);
		AddVertex(end, color);
	}
}

void Pu::DebugRenderer::AddBox(const AABB & box, Color color)
{
	const Vector3 ftl = box[0];
	const Vector3 ftr = box[1];
	const Vector3 fbr = box[2];
	const Vector3 fbl = box[3];
	const Vector3 btl = box[4];
	const Vector3 btr = box[5];
	const Vector3 bbr = box[6];
	const Vector3 bbl = box[7];

	AddLine(ftl, ftr, color);
	AddLine(ftr, fbr, color);
	AddLine(fbr, fbl, color);
	AddLine(fbl, ftl, color);

	AddLine(btl, btr, color);
	AddLine(btr, bbr, color);
	AddLine(bbr, bbl, color);
	AddLine(bbl, btl, color);

	AddLine(ftl, btl, color);
	AddLine(ftr, btr, color);
	AddLine(fbr, bbr, color);
	AddLine(fbl, bbl, color);
}

void Pu::DebugRenderer::AddBox(const AABB & box, const Matrix & transform, Color color)
{
	const Vector3 ftl = transform * box[0];
	const Vector3 ftr = transform * box[1];
	const Vector3 fbr = transform * box[2];
	const Vector3 fbl = transform * box[3];
	const Vector3 btl = transform * box[4];
	const Vector3 btr = transform * box[5];
	const Vector3 bbr = transform * box[6];
	const Vector3 bbl = transform * box[7];

	AddLine(ftl, ftr, color);
	AddLine(ftr, fbr, color);
	AddLine(fbr, fbl, color);
	AddLine(fbl, ftl, color);

	AddLine(btl, btr, color);
	AddLine(btr, bbr, color);
	AddLine(bbr, bbl, color);
	AddLine(bbl, btl, color);

	AddLine(ftl, btl, color);
	AddLine(ftr, btr, color);
	AddLine(fbr, bbr, color);
	AddLine(fbl, bbl, color);
}

void Pu::DebugRenderer::Render(CommandBuffer & cmdBuffer, const Matrix & projection, const Matrix & view)
{
	/* Only render if the pipeline is loaded. */
	if (CanBegin() && size)
	{
		cmdBuffer.AddLabel("Debug Renderer", Color::CodGray());

		/* Update the dynamic buffer. */
		buffer->BeginMemoryTransfer();
		memcpy(buffer->GetHostMemory(), queue, sizeof(ColoredVertex3D) * size);
		buffer->EndMemoryTransfer();
		buffer->Update(cmdBuffer);

		/* Update the descriptor. */
		uniforms->SetProjection(projection);
		uniforms->SetView(view);
		uniforms->Update(cmdBuffer);

		/* Render the debug lines. */
		Begin(cmdBuffer);
		if (dynamicLineWidth) cmdBuffer.SetLineWidth(lineWidth);
		cmdBuffer.BindGraphicsDescriptor(*uniforms);
		cmdBuffer.BindVertexBuffer(0, *bufferView);
		cmdBuffer.Draw(size, 1, 0, 0);
		End();

		cmdBuffer.EndLabel();
		size = 0;
	}
}

void Pu::DebugRenderer::OnPipelinePostInitialize(GraphicsPipeline & gfx)
{
	/* We only want to read from the depth buffer, debug information should not be writen to the depth buffer. */
	gfx.SetViewport(GetWindow().GetNative().GetClientBounds());
	gfx.SetTopology(PrimitiveTopology::LineList);
	if (depthBuffer) gfx.SetDepthStencilMode(true, false, false);
	gfx.AddVertexBinding<ColoredVertex3D>(0);

	/* Set the line width as a static parameter is possible, otherwise enable the dynamic line width. */
	if (dynamicLineWidth) gfx.AddDynamicState(DynamicState::LineWidth);
	else gfx.SetLineWidth(lineWidth);

	gfx.Finalize(0);
	RecreateFramebuffers(GetWindow(), gfx.GetRenderpass());
	uniforms = new DebugRendererUniformBlock(gfx);
}

void Pu::DebugRenderer::OnRenderpassLinkComplete(Renderpass & renderpass)
{
	Output &output = renderpass.GetOutput("FragColor");
	output.SetDescription(GetWindow().GetSwapchain());
	output.SetLoadOperation(AttachmentLoadOp::Load);

	/* 
	Only add the depth buffer if needed,
	the initial layout will probably be read-write so just leave it 
	but change the in-flight layout to read-only because we don't plan on writing to it
	*/
	if (depthBuffer)
	{
		Output &depth = renderpass.AddDepthStencil();
		depth.SetDescription(*depthBuffer);
		depth.SetLayout(ImageLayout::DepthStencilReadOnlyOptimal);
		depth.SetLoadOperation(AttachmentLoadOp::Load);
	}

	renderpass.GetAttribute("Color").SetOffset(vkoffsetof(ColoredVertex3D, Color));
}

void Pu::DebugRenderer::RecreateFramebuffers(GameWindow & window, const Renderpass & renderpass)
{
	if (depthBuffer) window.CreateFrameBuffers(renderpass, { &depthBuffer->GetView() });
	else window.CreateFrameBuffers(renderpass);
}

void Pu::DebugRenderer::AddVertex(Vector3 p, Color c)
{
	queue[size].Position = p;
	queue[size++].Color = c;
}