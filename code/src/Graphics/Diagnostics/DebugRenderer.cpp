#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DebugRenderer::DebugRenderer(GameWindow & window, AssetFetcher & loader, const DepthBuffer * depthBuffer, float lineWidth)
	: loader(loader), wnd(window), pipeline(nullptr), size(0), culled(0), lineWidth(lineWidth), depthBuffer(depthBuffer), thrown(false)
{
	/* We need a dynamic buffer because the data will update every frame, the queue is a raw array to increase performance. */
	buffer = new DynamicBuffer(window.GetDevice(), sizeof(ColoredVertex3D) * MaxDebugRendererVertices, BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer);
	bufferView = new BufferView(*buffer, sizeof(ColoredVertex3D));
	queue = reinterpret_cast<ColoredVertex3D*>(malloc(sizeof(ColoredVertex3D) * MaxDebugRendererVertices));

	renderpass = &loader.FetchRenderpass({ { L"{Shaders}VertexColor.vert.spv", L"{Shaders}VertexColor.frag.spv" } });
	renderpass->PreCreate.Add(*this, &DebugRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DebugRenderer::InitializePipeline);
	window.SwapchainRecreated.Add(*this, &DebugRenderer::SwapchainRecreated);

	/* We might need to manually initialize the pipeline because the renderpass might already be loaded. */
	if (renderpass->IsLoaded()) InitializePipeline(*renderpass);
}

Pu::DebugRenderer::~DebugRenderer(void)
{
	if (pipeline) delete pipeline;

	loader.Release(*renderpass);

	delete buffer;
	delete bufferView;
	delete queue;
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
	else culled += 2;
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

void Pu::DebugRenderer::AddRectangle(Vector3 lower, Vector3 upper, Color color)
{
	const Vector3 v{ upper.X, lower.Y, upper.Z };
	const Vector3 w{ lower.X, upper.Y, lower.Z };

	AddLine(lower, v, color);
	AddLine(lower, w, color);
	AddLine(upper, v, color);
	AddLine(upper, w, color);
}

void Pu::DebugRenderer::AddFrustum(const Frustum & frustum, Color color)
{
	const Vector3 nbl = Plane::IntersectionPoint(frustum.Near(), frustum.Left(), frustum.Bottom());
	const Vector3 ntr = Plane::IntersectionPoint(frustum.Near(), frustum.Right(), frustum.Top());
	const Vector3 fbl = Plane::IntersectionPoint(frustum.Far(), frustum.Left(), frustum.Bottom());
	const Vector3 ftr = Plane::IntersectionPoint(frustum.Far(), frustum.Right(), frustum.Top());
	const Vector3 nbr{ ntr.X, nbl.Y, ntr.Z };
	const Vector3 ntl{ nbl.X, ntr.Y, nbl.Z };
	const Vector3 fbr{ ftr.X, fbl.Y, ftr.Z };
	const Vector3 ftl{ fbl.X, ftr.Y, fbl.Z };

	AddLine(nbl, nbr, color);
	AddLine(nbl, ntl, color);
	AddLine(ntr, nbr, color);
	AddLine(ntr, ntl, color);

	AddLine(fbl, fbr, color);
	AddLine(fbl, ftl, color);
	AddLine(ftr, fbr, color);
	AddLine(ftr, ftl, color);

	AddLine(nbl, fbl, color);
	AddLine(nbr, fbr, color);
	AddLine(ntl, ftl, color);
	AddLine(ntr, ftr, color);
}

void Pu::DebugRenderer::Render(CommandBuffer & cmdBuffer, const Matrix & projection, const Matrix & view, bool clearBuffer)
{
	/* Make sure we cannot access garbage memory. */
	if (pipeline)
	{
		/* Only render if the pipeline is loaded. */
		if (pipeline->IsUsable() && size)
		{
			cmdBuffer.AddLabel("Debug Renderer", Color::CodGray());

			/* Update the dynamic buffer. */
			buffer->BeginMemoryTransfer();
			memcpy(buffer->GetHostMemory(), queue, sizeof(ColoredVertex3D) * size);
			buffer->EndMemoryTransfer();
			buffer->Update(cmdBuffer);

			/* Begin the renderpass. */
			cmdBuffer.BeginRenderPass(*renderpass, wnd.GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
			cmdBuffer.BindGraphicsPipeline(*pipeline);
			if (dynamicLineWidth) cmdBuffer.SetLineWidth(lineWidth);

			/* Update the descriptors. */
			const Matrix constants[2] = { projection, view };
			cmdBuffer.PushConstants(*renderpass, ShaderStageFlag::Vertex, sizeof(Matrix) << 1, constants);

			/* Render the debug lines. */
			cmdBuffer.BindVertexBuffer(0, *bufferView);
			cmdBuffer.Draw(size, 1, 0, 0);

			/* End the renderpass. */
			cmdBuffer.EndRenderPass();
			cmdBuffer.EndLabel();
		}
	}

	/* Clear the buffer to make sure that we don't get shapes from previous calls in one draw batch. */
	if (clearBuffer) size = 0;

	/* Log and warning if the user exceeds their limit once. */
	if (culled)
	{
		if (!thrown)
		{
			thrown = true;
			Log::Warning("Unable to render %u debug lines, consider upgrading the MaxDebugRendererVertices.", culled);
		}
		culled = 0;
	}
}

void Pu::DebugRenderer::AddVertex(Vector3 p, Color c)
{
	queue[size].Position = p;
	queue[size++].Color = c;
}

void Pu::DebugRenderer::InitializeRenderpass(Renderpass &)
{
	/* There is only one subpass that we need to initialize. */
	Subpass &subpass = renderpass->GetSubpass(0);

	Output &output = subpass.GetOutput("FragColor");
	output.SetDescription(wnd.GetSwapchain());
	output.SetLoadOperation(AttachmentLoadOp::Load);

	/*
	Only add the depth buffer if needed,
	the initial layout will probably be read-write so just leave it
	but change the in-flight layout to read-only because we don't plan on writing to it
	*/
	if (depthBuffer)
	{
		Output &depth = subpass.AddDepthStencil();
		depth.SetDescription(*depthBuffer);
		depth.SetLayout(ImageLayout::DepthStencilReadOnlyOptimal);
		depth.SetLoadOperation(AttachmentLoadOp::Load);
	}

	subpass.GetAttribute("Color").SetOffset(vkoffsetof(ColoredVertex3D, Color));
}

void Pu::DebugRenderer::InitializePipeline(Renderpass &)
{
	/* Initialize the pipeline. */
	pipeline = new GraphicsPipeline(*renderpass, 0);
	pipeline->SetViewport(wnd.GetNative().GetClientBounds());
	pipeline->SetTopology(PrimitiveTopology::LineList);
	pipeline->AddVertexBinding<ColoredVertex3D>(0);

	if (depthBuffer) pipeline->EnableDepthTest(false, CompareOp::LessOrEqual);

	/* Only use dynamic line width if the hardware cannot handle static one. */
	if (!pipeline->SetLineWidth(lineWidth))
	{
		dynamicLineWidth = true;
		pipeline->AddDynamicState(DynamicState::LineWidth);
	}

	/*
	Create the first framebuffers and finalize the pipeline.
	Only add the depth buffer to the frame buffers if needed.
	*/
	if (depthBuffer) wnd.CreateFrameBuffers(*renderpass, { &depthBuffer->GetView() });
	else wnd.CreateFrameBuffers(*renderpass);
	pipeline->Finalize();
}

void Pu::DebugRenderer::SwapchainRecreated(const GameWindow &)
{
	if (renderpass->IsLoaded())
	{
		/* Release all the resources that work with the renderpass. */
		delete pipeline;
		pipeline = nullptr;
		loader.Release(*renderpass);

		/* Reload the renderpass. */
		renderpass = &loader.FetchRenderpass({ { L"{Shaders}VertexColor.vert.spv", L"{Shaders}VertexColor.frag.spv" } });
		renderpass->PreCreate.Add(*this, &DebugRenderer::InitializeRenderpass);
		renderpass->PostCreate.Add(*this, &DebugRenderer::InitializePipeline);
		if (renderpass->IsLoaded()) InitializePipeline(*renderpass);
	}
}