#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Graphics/VertexLayouts/ColoredVertex3D.h"
#include "Graphics/Resources/DynamicBuffer.h"
#include "Graphics/Diagnostics/QueryChain.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Math/Interpolation.h"

#define AddLineInternal(p, q, clr)		AddVertex((p), (clr)); AddVertex((q), (clr))
#define CapacityEarlyOut(cap)			if (CheckCapacity(cap)) return

Pu::DebugRenderer::DebugRenderer(GameWindow & window, AssetFetcher & loader, const DepthBuffer * depthBuffer, float lineWidth)
	: loader(loader), wnd(window), pipeline(nullptr), size(0), culled(0),
	lineWidth(lineWidth), depthBuffer(depthBuffer), thrown(false), invalidated(0)
{
	/* We need a dynamic buffer because the data will update every frame, the queue is a raw array to increase performance. */
	buffer = new DynamicBuffer(window.GetDevice(), sizeof(ColoredVertex3D) * MaxDebugRendererVertices, BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer);
	queue = reinterpret_cast<ColoredVertex3D*>(malloc(sizeof(ColoredVertex3D) * MaxDebugRendererVertices));
	query = new QueryChain(window.GetDevice(), QueryType::Timestamp, 1);

	renderpass = &loader.FetchRenderpass({ { L"{Shaders}VertexColor.vert.spv", L"{Shaders}VertexColor.frag.spv" } });
	renderpass->PreCreate.Add(*this, &DebugRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DebugRenderer::InitializePipeline);
	window.SwapchainRecreated.Add(*this, &DebugRenderer::SwapchainRecreated);

	/* We might need to manually initialize the pipeline because the renderpass might already be loaded. */
	if (renderpass->IsLoaded()) InitializePipeline(*renderpass);

	/* Precalculate the sines and cosines that will be used. */
	for (uint32 i = 0; i < END_ANGLE; i++)
	{
		const float theta = i * (TAU / END_ANGLE);
		sines[i] = sinf(theta);
		cosines[i] = cosf(theta);
	}
}

Pu::DebugRenderer::~DebugRenderer(void)
{
	if (pipeline) delete pipeline;

	loader.Release(*renderpass);

	delete query;
	delete buffer;
	delete queue;
}

void Pu::DebugRenderer::AddLine(const Line & line, Color color)
{
	AddLine(line.Start, line.End, color);
}

void Pu::DebugRenderer::AddLine(Vector3 start, Vector3 end, Color color)
{
	CapacityEarlyOut(2);
	AddVertex(start, color);
	AddVertex(end, color);
}

void Pu::DebugRenderer::AddBezier(Vector3 start, Vector3 control, Vector3 end, Color color, uint32 segments)
{
	CapacityEarlyOut(segments << 1);

	Vector3 a = start;
	const float step = recip(static_cast<float>(segments));

	for (float v = 0.0f; v < 1.0f; v += step)
	{
		const Vector3 b = cubic(start, control, end, v);
		AddLineInternal(a, b, color);
		a = b;
	}
}

void Pu::DebugRenderer::AddBezier(Vector3 start, Vector3 control1, Vector3 control2, Vector3 end, Color color, uint32 segments)
{
	CapacityEarlyOut(segments << 1);

	Vector3 a = start;
	const float step = recip(static_cast<float>(segments));

	for (float v = 0.0f; v < 1.0f; v += step)
	{
		const Vector3 b = quadratic(start, control1, control2, end, v);
		AddLineInternal(a, b, color);
		a = b;
	}
}

void Pu::DebugRenderer::AddSpline(const Spline & spline, Color color, uint32 segments)
{
	CapacityEarlyOut(segments << 1);

	Vector3 a = spline.GetLocation(0.0f);
	const float step = recip(static_cast<float>(segments));

	for (float v = 0.0f; v < 1.0f; v += step)
	{
		const Vector3 b = spline.GetLocation(v);
		AddLineInternal(a, b, color);
		a = b;
	}
}

void Pu::DebugRenderer::AddArrow(Vector3 start, Vector3 direction, Color color, float length, float headAngle)
{
	CapacityEarlyOut(6);

	/* The length of the arrow head is always relative to the arrow shaft length. */
	const float headLength = length * recip(4.0f);
	const Quaternion base = Quaternion::Create(direction, tangent(direction));

	/* Calculate the directions of the arrow head lines. */
	const Vector3 right = base * Quaternion::Create(PI + headAngle, Vector3{ 1.0f, 0.0f,0.0f }) * Vector3::Forward();
	const Vector3 left = reflect(-right, direction);

	/* Calculate the end points for the lines. */
	const Vector3 head = start + direction * length;
	const Vector3 end1 = head + right * headLength;
	const Vector3 end2 = head + left * headLength;

	/* Add the shaft line and head lines. */
	AddLineInternal(start, head, color);
	AddLineInternal(head, end1, color);
	AddLineInternal(head, end2, color);
}

void Pu::DebugRenderer::AddTransform(const Matrix & transform, float scale, Vector3 offset)
{
	const Vector3 start = transform.GetTranslation() + offset;
	const Vector3 right = transform.GetRight();
	const Vector3 up = transform.GetUp();
	const Vector3 forward = transform.GetForward();

	const float rl = right.Length();
	const float ul = up.Length();
	const float fl = forward.Length();

	AddArrow(start, right / rl, Color::Red(), rl * scale);
	AddArrow(start, up / ul, Color::Green(), ul * scale);
	AddArrow(start, forward / fl, Color::Blue(), fl * scale);
}

void Pu::DebugRenderer::AddBox(const AABB & box, Color color)
{
	CapacityEarlyOut(24);

	const Vector3 ftl = box[0];
	const Vector3 ftr = box[1];
	const Vector3 fbr = box[2];
	const Vector3 fbl = box[3];
	const Vector3 btl = box[4];
	const Vector3 btr = box[5];
	const Vector3 bbr = box[6];
	const Vector3 bbl = box[7];

	/* Front face */
	AddLineInternal(ftl, ftr, color);
	AddLineInternal(ftr, fbr, color);
	AddLineInternal(fbr, fbl, color);
	AddLineInternal(fbl, ftl, color);

	/* Back face */
	AddLineInternal(btl, btr, color);
	AddLineInternal(btr, bbr, color);
	AddLineInternal(bbr, bbl, color);
	AddLineInternal(bbl, btl, color);

	/* Inbetween lines */
	AddLineInternal(ftl, btl, color);
	AddLineInternal(ftr, btr, color);
	AddLineInternal(fbr, bbr, color);
	AddLineInternal(fbl, bbl, color);
}

void Pu::DebugRenderer::AddBox(const AABB & box, const Matrix & transform, Color color)
{
	CapacityEarlyOut(24);

	const Vector3 ftl = transform * box[0];
	const Vector3 ftr = transform * box[1];
	const Vector3 fbr = transform * box[2];
	const Vector3 fbl = transform * box[3];
	const Vector3 btl = transform * box[4];
	const Vector3 btr = transform * box[5];
	const Vector3 bbr = transform * box[6];
	const Vector3 bbl = transform * box[7];

	/* Front face */
	AddLineInternal(ftl, ftr, color);
	AddLineInternal(ftr, fbr, color);
	AddLineInternal(fbr, fbl, color);
	AddLineInternal(fbl, ftl, color);

	/* Back face */
	AddLineInternal(btl, btr, color);
	AddLineInternal(btr, bbr, color);
	AddLineInternal(bbr, bbl, color);
	AddLineInternal(bbl, btl, color);

	/* Inbetween lines */
	AddLineInternal(ftl, btl, color);
	AddLineInternal(ftr, btr, color);
	AddLineInternal(fbr, bbr, color);
	AddLineInternal(fbl, bbl, color);
}

void Pu::DebugRenderer::AddSphere(Sphere sphere, Color color)
{
	AddEllipsoid(sphere.Center, sphere.Radius, sphere.Radius, sphere.Radius, color);
}

void Pu::DebugRenderer::AddSphere(Vector3 center, float radius, Color color)
{
	AddEllipsoid(center, radius, radius, radius, color);
}

void Pu::DebugRenderer::AddEllipsoid(Vector3 center, float xRadius, float yRadius, float zRadius, Color color)
{
	CapacityEarlyOut(sqr(END_ANGLE) << 1);

	/*
	Ellipsoid is a UV sphere with different radii for each dimension.
	We have to increase the amount if divisions by two because we count one div as a full circle.
	*/
	const Vector3 scalar{ xRadius, yRadius, zRadius };

	/* Loop through all parallels. */
	for (uint32 theta = 0; theta < END_ANGLE; theta++)
	{
		const float ct = cosines[theta];
		const float st = sines[theta];

		/* Start point for the parallel and meridian. */
		Vector3 startP = center + Vector3(st, ct, 0.0f) * scalar;
		Vector3 startM = center + Vector3(ct, 0.0f, st) * scalar;

		/* Create one meridian per parallel line. */
		for (uint32 phi = 0; phi < END_ANGLE; phi++)
		{
			const float cp = cosines[phi];
			const float sp = sines[phi];

			const Vector3 endM = center + Vector3(cp * ct, sp, st * cp) * scalar;
			AddLineInternal(startM, endM, color);
			startM = endM;

			const Vector3 endP = center + Vector3(cp * st, ct, sp * st) * scalar;
			AddLineInternal(startP, endP, color);
			startP = endP;
		}
	}
}

void Pu::DebugRenderer::AddCapsule(Vector3 center, float height, float radius, Color color)
{
	CapacityEarlyOut((END_ANGLE << 3) + 4);

	/*
	The capsule is rounded at both ends so use the ellipsoid divisions to get the preferred amount of rounding.
	The capsule height includes the hemispheres on the top and bottom, so remove those radii from the ring height offset.
	*/
	const float offset = height - radius;
	const Vector3 ringOffset = Vector3(0.0f, offset, 0.0f);

	const Vector3 originalX = center + Vector3(0.0f, offset, radius);
	const Vector3 originalZ = center + Vector3(radius, offset, 0.0f);

	Vector3 startX = originalX;
	Vector3 startY = center + Vector3(radius, 0.0f, 0.0f);
	Vector3 startZ = originalZ;

	for (uint32 theta = 0; theta < END_ANGLE; theta++)
	{
		const float ct = cosines[theta];
		const float st = sines[theta];

		/* We'll switch from top to bottom hemisphere halfway through the loop. */
		const Vector3 adder = theta < EllipsiodDivs ? ringOffset : -ringOffset;
		const Vector3 endX = center + Vector3(0.0f, st, ct) * radius + adder;
		const Vector3 endY = center + Vector3(ct, 0.0f, st) * radius;
		const Vector3 endZ = center + Vector3(ct, st, 0.0f) * radius + adder;

		/*
		Draw the hemisphere for the X and Z axis.
		Also add the top and bottom rings.
		*/
		AddLineInternal(startX, endX, color);
		AddLineInternal(startY + ringOffset, endY + ringOffset, color);
		AddLineInternal(startY - ringOffset, endY - ringOffset, color);
		AddLineInternal(startZ, endZ, color);

		startX = endX;
		startY = endY;
		startZ = endZ;
	}

	/*
	The above loop will add 2 lines from the center cylinder, but not all 4.
	So we add those here.
	*/
	AddLineInternal(startX, originalX, color);
	AddLineInternal(startZ, originalZ, color);
}

void Pu::DebugRenderer::AddRectangle(Vector3 lower, Vector3 upper, Color color)
{
	CapacityEarlyOut(8);

	const Vector3 v{ upper.X, lower.Y, upper.Z };
	const Vector3 w{ lower.X, upper.Y, lower.Z };

	AddLineInternal(lower, v, color);
	AddLineInternal(lower, w, color);
	AddLineInternal(upper, v, color);
	AddLineInternal(upper, w, color);
}

void Pu::DebugRenderer::AddFrustum(const Frustum & frustum, Color color)
{
	CapacityEarlyOut(24);

	/* We calculate 8 corner points by intersecting the frustum bounds. */
	const Vector3 nbl = Plane::IntersectionPoint(frustum.Near(), frustum.Left(), frustum.Bottom());
	const Vector3 ntr = Plane::IntersectionPoint(frustum.Near(), frustum.Right(), frustum.Top());
	const Vector3 fbl = Plane::IntersectionPoint(frustum.Far(), frustum.Left(), frustum.Bottom());
	const Vector3 ftr = Plane::IntersectionPoint(frustum.Far(), frustum.Right(), frustum.Top());
	const Vector3 nbr = Plane::IntersectionPoint(frustum.Near(), frustum.Right(), frustum.Bottom());
	const Vector3 ntl = Plane::IntersectionPoint(frustum.Near(), frustum.Left(), frustum.Top());
	const Vector3 fbr = Plane::IntersectionPoint(frustum.Far(), frustum.Right(), frustum.Bottom());
	const Vector3 ftl = Plane::IntersectionPoint(frustum.Far(), frustum.Left(), frustum.Top());

	/* Near plane */
	AddLineInternal(nbl, nbr, color);
	AddLineInternal(nbl, ntl, color);
	AddLineInternal(ntr, nbr, color);
	AddLineInternal(ntr, ntl, color);

	/* Far plane */
	AddLineInternal(fbl, fbr, color);
	AddLineInternal(fbl, ftl, color);
	AddLineInternal(ftr, fbr, color);
	AddLineInternal(ftr, ftl, color);

	/* Inbetween lines */
	AddLineInternal(nbl, fbl, color);
	AddLineInternal(nbr, fbr, color);
	AddLineInternal(ntl, ftl, color);
	AddLineInternal(ntr, ftr, color);
}

void Pu::DebugRenderer::Render(CommandBuffer & cmdBuffer, const Camera & camera, bool clearBuffer)
{
	/* Make sure we cannot access garbage memory. */
	if (pipeline && !invalidated)
	{
		/* Only render if the pipeline is loaded. */
		if (pipeline->IsUsable() && size)
		{
			Profiler::Add("Debug Renderer", Color::Orange(), query->GetProfilerTimeDelta(0));
			cmdBuffer.AddLabel("Debug Renderer", Color::CodGray());
			query->Reset(cmdBuffer);

			/* Update the dynamic buffer. */
			buffer->BeginMemoryTransfer();
			memcpy(buffer->GetHostMemory(), queue, sizeof(ColoredVertex3D) * size);
			buffer->EndMemoryTransfer();
			buffer->Update(cmdBuffer);

			/* Begin the renderpass. */
			cmdBuffer.BeginRenderPass(*renderpass, wnd.GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
			query->RecordTimestamp(cmdBuffer, 0, PipelineStageFlag::TopOfPipe);
			cmdBuffer.BindGraphicsPipeline(*pipeline);
			if (dynamicLineWidth) cmdBuffer.SetLineWidth(lineWidth);

			/* Update the descriptors. */
			const Matrix constants[2] = { camera.GetProjection(), camera.GetView() };
			cmdBuffer.PushConstants(*pipeline, ShaderStageFlag::Vertex, 0, sizeof(Matrix) << 1, constants);

			/* Render the debug lines. */
			cmdBuffer.BindVertexBuffer(0, *buffer, 0);
			cmdBuffer.Draw(size, 1, 0, 0);

			/* End the renderpass. */
			query->RecordTimestamp(cmdBuffer, 0, PipelineStageFlag::BottomOfPipe);
			cmdBuffer.EndRenderPass();
			cmdBuffer.EndLabel();
		}
	}

	/* Clear the buffer to make sure that we don't get shapes from previous calls in one draw batch. */
	if (clearBuffer) size = 0;

	/* Log and warning if the user exceeds their limit once. */
#ifdef _DEBUG
	if (culled)
	{
		if (!thrown)
		{
			thrown = true;
			Log::Warning("Unable to render %u debug lines, consider upgrading the MaxDebugRendererVertices.", culled);
		}
		culled = 0;
	}
#endif
}

#pragma warning(push)
#pragma warning(disable:4458)
void Pu::DebugRenderer::Reset(const DepthBuffer & depthBuffer)
{
	/*
	Reset the depth buffer and recreate the framebuffers if needed.
	The depth buffer might not have been invalidated yet, if this is the case;
	just set the new depthbuffer and mark it as invalid.
	*/
	if (invalidated)
	{
		this->depthBuffer = &depthBuffer;
		CreateFrameBuffers();
		invalidated = 0;
	}
	else
	{
		this->depthBuffer = &depthBuffer;
		invalidated = 2;
	}
}
#pragma warning(pop)

bool Pu::DebugRenderer::CheckCapacity(uint32 addition) const
{
#ifdef _DEBUG
	if (size + addition <= MaxDebugRendererVertices) return false;

	culled += addition;
	return true;
#else
	retun size + addition > MaxDebugRendererVertices;
#endif
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
	CreatePipeline();
	CreateFrameBuffers();
}

void Pu::DebugRenderer::CreatePipeline(void)
{
	/* Delete the old pipeline if needed. */
	if (!pipeline) pipeline = new GraphicsPipeline(*renderpass, 0);

	/* Initialize the pipeline. */
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

	pipeline->Finalize();
}

void Pu::DebugRenderer::CreateFrameBuffers(void)
{
	/*
	Create the first framebuffers and finalize the pipeline.
	Only add the depth buffer to the frame buffers if needed.
	*/
	if (depthBuffer) wnd.CreateFramebuffers(*renderpass, { &depthBuffer->GetView() });
	else wnd.CreateFramebuffers(*renderpass);
}

void Pu::DebugRenderer::SwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs & args)
{
	/*
	Invalidated can have 3 states:
	0: The debug renderer is good to use
	1: The debug renderer was invalidated by the spawchain being recreated
	2: The debug renderer was invalidated by the user assigning a new depth buffer

	We only need to recreate the renderpass if the format changed, the depth buffer will not change.
	But we need to invalidate the debug renderer if the area changed, this is because the depth buffer will need to be recreated.
	So if we're using a depth buffer then invalidate the debug renderer and wait for the user to set a new depth buffer, otherwise;
	Just recreate the framebuffer.

	We could set the viewport and scissor as dynamic state, but I opted to just recreate the pipeline if the area changes.
	*/
	if (renderpass->IsLoaded())
	{
		if (args.FormatChanged) renderpass->Recreate();
		else if (args.AreaChanged)
		{
			CreatePipeline();
			if (depthBuffer && !invalidated) invalidated = 1;
			else if (invalidated != 1) CreateFrameBuffers();
		}
	}
}