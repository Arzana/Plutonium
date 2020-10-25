#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Graphics/Diagnostics/ProfilerChain.h"
#include "Graphics/Resources/DynamicBuffer.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Core/Diagnostics/Profiler.h"

using namespace Pu;

constexpr DeviceSize Line_START = 0;
constexpr DeviceSize Arrow_START = MaxDebugRendererObjects;
constexpr DeviceSize Box_START = MaxDebugRendererObjects << 1;
constexpr DeviceSize Ellipsoid_START = MaxDebugRendererObjects * 3;
constexpr DeviceSize Hemisphere_START = MaxDebugRendererObjects << 2;
constexpr DeviceSize HOST_BUFFER_COUNT = MaxDebugRendererObjects * 5;
constexpr uint32 TRIG_BUFFER_SIZE = EllipsiodDivs << 1;

#define GetOffset(list)					VertexLayout *mem = (reinterpret_cast<VertexLayout*>(buffer->GetHostMemory()) + ##list##_START + cnt##list)
#define CapacityEarlyOut(amnt, list)	if (CheckCapacity(list, amnt)) return
#define TryStart(amnt, list)			if (CheckCapacity(cnt##list, amnt)) { return; } GetOffset(list)
#define cpy_line(p, q)					*mem++ = (p); *mem++ = (q)

class LoadDebugShapesTask
	: public Task
{
public:
	LoadDebugShapesTask(AssetLoader &loader, MeshCollection *result)
		: Task("Generate Debug Shapes"), loader(loader), result(*result), vrtxIdx(1)
	{
		src = new StagingBuffer(loader.GetDevice(), GPU_BUFFER_SIZE * sizeof(Vector3));
		result->AddView(0u, GPU_BUFFER_SIZE * sizeof(Vector3));
	}

	Result Execute(void) final
	{
		/* Precalculate the sines and cosines that we'll use. */
		for (uint32 i = 0; i < TRIG_BUFFER_SIZE; i++)
		{
			const float theta = i * (TAU / TRIG_BUFFER_SIZE);
			sines[i] = sinf(theta);
			cosines[i] = cosf(theta);
		}

		src->BeginMemoryTransfer();
		Vector3 *mem = reinterpret_cast<Vector3*>(src->GetHostMemory());

		/* Add the line primitive. */
		cpy_line(Vector3(), Vector3::Forward());
		result.AddMesh(Mesh{ 0u, 0u, sizeof(Vector3), 2 });

		/* Add the arrow primitive. */
		const Vector3 head = Vector3::Forward();
		const Vector3 right = Quaternion::CreatePitch(PI + PI8) * Vector3::Forward() * 0.25f;

		cpy_line(Vector3(), head);
		cpy_line(head, head + right);
		cpy_line(head, head + reflect(-right, Vector3::Forward()));
		AddMesh(6);

		/* Add the box. */
		cpy_line(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f));
		cpy_line(Vector3(0.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f));
		cpy_line(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f));
		cpy_line(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));

		cpy_line(Vector3(1.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
		cpy_line(Vector3(1.0f, 1.0f, 1.0f), Vector3(1.0f, 0.0f, 1.0f));
		cpy_line(Vector3(1.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f));
		cpy_line(Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f));

		cpy_line(Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f));
		cpy_line(Vector3(0.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f));
		cpy_line(Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 1.0f));
		cpy_line(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
		AddMesh(24);

		/* Add the ellipsoid. */
		for (uint32 theta = 0; theta < TRIG_BUFFER_SIZE; theta++)
		{
			const float ct = cosines[theta];
			const float st = sines[theta];

			Vector3 startP{ st, ct, 0.0f };
			Vector3 startM{ ct, 0.0f, st };

			for (uint32 phi = 0; phi < TRIG_BUFFER_SIZE; phi++)
			{
				const float cp = cosines[phi];
				const float sp = sines[phi];

				const Vector3 endM{ cp * ct, sp, st * cp };
				cpy_line(startM, endM);
				startM = endM;

				const Vector3 endP{ cp * st, ct, sp * st };
				cpy_line(startP, endP);
				startP = endP;
			}
		}
		AddMesh(sqr(TRIG_BUFFER_SIZE) << 2);

		/* Add the hemisphere. */
		for (uint32 theta = 0; theta < TRIG_BUFFER_SIZE; theta++)
		{
			const float ct = cosines[theta];
			const float st = sines[theta];
			if (ct < -0.001f) continue;

			Vector3 startM{ ct, 0.0f, st };
			Vector3 startP{ st, ct, 0.0f };

			for (uint32 phi = 0; phi < (TRIG_BUFFER_SIZE >> 1) + 1; phi++)
			{
				const float cp = cosines[phi];
				const float sp = sines[phi];

				const Vector3 endP{ cp * st, ct, sp * st };
				cpy_line(startP, endP);
				startP = endP;

				const Vector3 endM{ cp * ct, sp, st * cp };
				cpy_line(startM, endM);
				startM = endM;
			}
		}
		AddMesh(sqr((TRIG_BUFFER_SIZE >> 1) + 1) << 2);

		/* Finalize the mesh and create the GPU buffer. */
		src->EndMemoryTransfer();
		result.Finalize(loader.GetDevice(), GPU_BUFFER_SIZE * sizeof(Vector3));
		loader.StageBuffer(*src, result.GetBuffer(), PipelineStageFlags::VertexInput, AccessFlags::VertexAttributeRead, L"Debug Renderer Meshes");
		return Result::AutoDelete();
	}

private:
	constexpr static DeviceSize GPU_BUFFER_SIZE =
		2 +											// Line
		6 +											// Arrow
		24 +										// Box
		(sqr(TRIG_BUFFER_SIZE) << 2) +				// Ellipsoid
		(sqr((TRIG_BUFFER_SIZE >> 1) + 1) << 2);	// Hemisphere

	AssetLoader &loader;
	StagingBuffer *src;
	MeshCollection &result;

	float cosines[TRIG_BUFFER_SIZE];
	float sines[TRIG_BUFFER_SIZE];
	uint32 vrtxIdx;

	void AddMesh(uint32 count)
	{
		result.AddMesh(Mesh{ 0u, vrtxIdx, sizeof(Vector3), count });
		vrtxIdx += count;
	}
};

Pu::DebugRenderer::DebugRenderer(GameWindow & window, AssetFetcher & loader, const DepthBuffer * depthBuffer, float lineWidth)
	: loader(loader), wnd(window), pipeline(nullptr), lineWidth(lineWidth),
	depthBuffer(depthBuffer), thrown(false), invalidated(0), meshes(new MeshCollection()),
	cntLine(0), cntArrow(0), cntBox(0), cntEllipsoid(0), cntHemisphere(0)
#ifdef _DEBUG
	, culled(0)
#endif
{
	/* We need a dynamic buffer because the data will update every frame. */
	buffer = new DynamicBuffer(window.GetDevice(), sizeof(VertexLayout) * HOST_BUFFER_COUNT, BufferUsageFlags::TransferDst | BufferUsageFlags::VertexBuffer);
	buffer->BeginMemoryTransfer();

	query = new ProfilerChain(window.GetDevice(), "Debug Renderer", Color::Orange());

	renderpass = &loader.FetchRenderpass(nullptr, { { L"{Shaders}VertexColor.vert.spv", L"{Shaders}VertexColor.frag.spv" } });
	renderpass->PreCreate.Add(*this, &DebugRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DebugRenderer::InitializePipeline);
	window.SwapchainRecreated.Add(*this, &DebugRenderer::SwapchainRecreated);

	/* We might need to manually initialize the pipeline because the renderpass might already be loaded. */
	if (renderpass->IsLoaded()) InitializePipeline(*renderpass);

	/* Create the primitives. */
	Task *task = new LoadDebugShapesTask(loader.GetLoader(), meshes);
	TaskScheduler::Spawn(*task);
}

Pu::DebugRenderer::~DebugRenderer(void)
{
	if (pipeline) delete pipeline;
	loader.Release(*renderpass);

	delete meshes;
	delete query;

	buffer->EndMemoryTransfer();
	delete buffer;
}

void Pu::DebugRenderer::AddLine(const Line & line, Color color)
{
	AddLine(line.Start, line.End, color);
}

void Pu::DebugRenderer::AddLine(Vector3 start, Vector3 end, Color color)
{
	CapacityEarlyOut(1, cntLine);
	AddLineInternal(start, end, color);
}

void Pu::DebugRenderer::AddBezier(Vector3 start, Vector3 control, Vector3 end, Color color, uint32 segments)
{
	CapacityEarlyOut(segments << 1, cntLine);

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
	CapacityEarlyOut(segments << 1, cntLine);

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
	CapacityEarlyOut(segments << 1, cntLine);

	Vector3 a = spline.GetLocation(0.0f);
	const float step = recip(static_cast<float>(segments));

	for (float v = 0.0f; v < 1.0f; v += step)
	{
		const Vector3 b = spline.GetLocation(v);
		AddLineInternal(a, b, color);
		a = b;
	}
}

void Pu::DebugRenderer::AddArrow(Vector3 start, Vector3 direction, Color color, float length)
{
	TryStart(1, Arrow);
	mem->mdl = Matrix::CreateWorld(start, Quaternion::Create(direction, tangent(direction)), length);
	mem->clr = color;
	++cntArrow;
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
	TryStart(1, Box);
	mem->mdl = Matrix::CreateScaledTranslation(box.LowerBound, box.GetSize());
	mem->clr = color;
	++cntBox;
}

void Pu::DebugRenderer::AddBox(const AABB & box, const Matrix & transform, Color color)
{
	TryStart(1, Box);
	mem->mdl = transform * Matrix::CreateScaledTranslation(box.LowerBound, box.GetSize());
	mem->clr = color;
	++cntBox;
}

void Pu::DebugRenderer::AddBox(const OBB & box, Color color)
{
	TryStart(1, Box);
	mem->mdl = Matrix::CreateWorld((box.Center - box.Orientation * box.Extent), box.Orientation, box.Extent * 2.0f);
	mem->clr = color;
	++cntBox;
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
	TryStart(1, Ellipsoid);
	mem->mdl = Matrix::CreateScaledTranslation(center, Vector3(xRadius, yRadius, zRadius));
	mem->clr = color;
	++cntEllipsoid;
}

void Pu::DebugRenderer::AddCapsule(Vector3 center, float height, float radius, Color color)
{
	/* Caps. */
	{
		TryStart(2, Hemisphere);

		mem->mdl = Matrix::CreateScaledTranslation(Vector3(center.X, center.Y + height * 0.5f, center.Z), radius);
		mem++->clr = color;

		mem->mdl = Matrix::CreateWorld(Vector3(center.X, center.Y - height * 0.5f, center.Z), Quaternion{ 0.0f, 1.0f, 0.0f, 0.0f }, radius);
		mem->clr = color;

		cntHemisphere += 2;
	}

	/* Cylinder. */
	{
		constexpr float delta = TAU / TRIG_BUFFER_SIZE;

		for (float theta = 0.0f; theta < TAU; theta += delta)
		{
			const float ct = cosf(theta);
			const float st = sinf(theta);
			const Vector3 start = center + Vector3(ct * radius, height * 0.5f, st * radius);
			const Vector3 end = center + Vector3(ct * radius, height * -0.5f, st * radius);
			AddLineInternal(start, end, color);
		}

		const float adder = radius / (TRIG_BUFFER_SIZE >> 3);
		for (float h = height * -0.5f + adder; h < height * 0.5f - adder; h += adder)
		{
			Vector3 startP = center + Vector3(cosf((TRIG_BUFFER_SIZE - 1) * delta) * radius, h, sinf((TRIG_BUFFER_SIZE - 1) * delta) * radius);
			for (float theta = 0.0f; theta < TAU; theta += delta)
			{
				const float ct = cosf(theta);
				const float st = sinf(theta);

				const Vector3 endP = center + Vector3(ct * radius, h, st * radius);
				AddLineInternal(startP, endP, color);
				startP = endP;
			}
		}
	}
}

void Pu::DebugRenderer::AddFrustum(const Frustum & frustum, Color color)
{
	CapacityEarlyOut(24, cntLine);

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
		if (pipeline->IsUsable() && meshes->IsUsable())
		{
			if (!(cntLine | cntArrow | cntBox | cntEllipsoid | cntHemisphere)) return;

			Profiler::Add(*query, cmdBuffer, true);
			cmdBuffer.AddLabel("Debug Renderer", Color::CodGray());

			/* Update the dynamic buffer. */
			buffer->Flush(WholeSize, 0);
			buffer->Update(cmdBuffer);

			/* Begin the renderpass. */
			cmdBuffer.BeginRenderPass(*renderpass, wnd.GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
			query->RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::TopOfPipe);
			cmdBuffer.BindGraphicsPipeline(*pipeline);
			if (dynamicLineWidth) cmdBuffer.SetLineWidth(lineWidth);

			/* Update the descriptors. */
			const Matrix constants[2] = { camera.GetProjection(), camera.GetView() };
			cmdBuffer.PushConstants(*pipeline, ShaderStageFlags::Vertex, 0, sizeof(Matrix) << 1, constants);

			/* Render the debug lines. */
			meshes->Bind(cmdBuffer, 0, 1);
			cmdBuffer.BindVertexBuffer(1, *buffer, 0);
			if (cntLine) meshes->GetMesh(0).Draw(cmdBuffer, Line_START, cntLine);
			if (cntArrow) meshes->GetMesh(1).Draw(cmdBuffer, Arrow_START, cntArrow);
			if (cntBox) meshes->GetMesh(2).Draw(cmdBuffer, Box_START, cntBox);
			if (cntEllipsoid) meshes->GetMesh(3).Draw(cmdBuffer, Ellipsoid_START, cntEllipsoid);
			if (cntHemisphere) meshes->GetMesh(4).Draw(cmdBuffer, Hemisphere_START, cntHemisphere);

			/* End the renderpass. */
			query->RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::BottomOfPipe);
			cmdBuffer.EndRenderPass();
			cmdBuffer.EndLabel();
		}
	}

	/* Clear the buffer to make sure that we don't get shapes from previous calls in one draw batch. */
	if (clearBuffer)
	{
		cntLine = 0;
		cntArrow = 0;
		cntBox = 0;
		cntEllipsoid = 0;
		cntHemisphere = 0;
	}

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

bool Pu::DebugRenderer::CheckCapacity(uint32 list, uint32 addition) const
{
#ifdef _DEBUG
	if (list + addition <= MaxDebugRendererObjects) return false;

	culled += addition;
	return true;
#else
	return list + addition > MaxDebugRendererObjects;
#endif
}

void Pu::DebugRenderer::AddLineInternal(Vector3 start, Vector3 end, Color color)
{
	GetOffset(Line);
	/*
	The model for the line is just two points ([0, 0, 0] and [0, 0, 1]).
	This code effectively translates that line to start start position,
	rotates it the the desired orientation and scales it (translation * rotation * scale).
	We can optimize this because the mesh is pretty much the z-axis.

	Translation is never altered by the matrix multiplication,
	so we just set that in the forth column.

	The desired rotation matrix would transform the mesh to the desired line.
	This is the same as rotating the z-axis to be the line.
	So we can substitute the third column with our line delta (z).

	Meanwhile transformation applied to the other axis (x and y) doesn't matter.
	Therefor we set it to the identity scalars to make sure it doesn't interfere.
	*/
	const Vector3 z = end - start;
	mem->mdl = Matrix{
		1.0f, 0.0f, z.X, start.X,
		0.0f, 1.0f, z.Y, start.Y,
		0.0f, 0.0f, z.Z, start.Z,
		0.0f, 0.0f, 0.0f, 1.0f };

	mem->clr = color;
	++cntLine;
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

	Attribute &clr = subpass.GetAttribute("Color");
	clr.SetOffset(vkoffsetof(VertexLayout, clr));
	clr.SetBinding(1);

	subpass.GetAttribute("Model").SetBinding(1);
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
	pipeline->AddVertexBinding<Vector3>(0);
	pipeline->AddVertexBinding<VertexLayout>(1, VertexInputRate::Instance);
	pipeline->SetLineRasterizationMode(LineRasterizationMode::Bresenham, false, 0, 0);

	if (depthBuffer) pipeline->EnableDepthTest(false, CompareOp::LessOrEqual);

	/* Only use dynamic line width if the hardware cannot handle static one. */
	if (!pipeline->SetLineWidth(lineWidth))
	{
		dynamicLineWidth = true;
		pipeline->AddDynamicState(DynamicState::LineWidth);
	}

	pipeline->Finalize();
	pipeline->SetDebugName("Debug Renderer");
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
		else if (args.ImagesInvalidated) CreateFrameBuffers();
	}
}