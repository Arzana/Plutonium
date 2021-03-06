#include "Graphics/Lighting/LightProbeRenderer.h"
#include "Graphics/Lighting/LightProbeUniformBlock.h"
#include "Graphics/VertexLayouts/Advanced3D.h"
#include "Graphics/Diagnostics/QueryChain.h"
#include "Core/Diagnostics/Profiler.h"

Pu::LightProbeRenderer::LightProbeRenderer(AssetFetcher & loader, uint32 maxProbeCount)
	: loader(&loader), gfx(nullptr), pool(nullptr), maxSets(maxProbeCount)
{
	/* Load the renderpass and initialize it if needed. */
	renderpass = &loader.FetchRenderpass(nullptr, { { L"{Shaders}LightProbe.vert.spv", L"{Shaders}LightProbe.geom.spv", L"{Shaders}LightProbe.frag.spv" } });
	renderpass->PreCreate.Add(*this, &LightProbeRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &LightProbeRenderer::InitializePipeline);
	if (renderpass->IsLoaded()) InitializePipeline(*renderpass);

	timer = new ProfilerChain(loader.GetDevice(), "Light Probe Update", Color::Blue());
}

Pu::LightProbeRenderer::LightProbeRenderer(LightProbeRenderer && value)
	: loader(value.loader), renderpass(value.renderpass), gfx(value.gfx),
	pool(value.pool), maxSets(value.maxSets), timer(value.timer)
{
	value.renderpass = nullptr;
	value.gfx = nullptr;
	value.pool = nullptr;
	value.timer = nullptr;
}

Pu::LightProbeRenderer & Pu::LightProbeRenderer::operator=(LightProbeRenderer && other)
{
	if (this != &other)
	{
		Destroy();

		loader = other.loader;
		renderpass = other.renderpass;
		gfx = other.gfx;
		pool = other.pool;
		maxSets = other.maxSets;
		timer = other.timer;

		other.renderpass = nullptr;
		other.gfx = nullptr;
		other.pool = nullptr;
		other.timer = nullptr;
	}

	return *this;
}

void Pu::LightProbeRenderer::Initialize(CommandBuffer & cmdBuffer)
{
	/* We need to initialize the transforms pool at least once. */
	pool->Update(cmdBuffer, PipelineStageFlags::GeometryShader);
	Profiler::Add(*timer, cmdBuffer, true);
}

void Pu::LightProbeRenderer::Start(LightProbe & probe, CommandBuffer & cmdBuffer) const
{
	/* Ingore baked light probes. */
	if (probe.cycleMode == LightProbe::CycleMode::Baked) return;
	probe.Lock();

	/* The image should be in a color attachment write access. */
	Profiler::Begin("Light Probes", Color::Blue());
	cmdBuffer.AddLabel("Light Probe Update", Color::Red());
	cmdBuffer.MemoryBarrier(*probe.image, PipelineStageFlags::FragmentShader, PipelineStageFlags::ColorAttachmentOutput, ImageLayout::ShaderReadOnlyOptimal, AccessFlags::ColorAttachmentWrite, probe.texture->GetFullRange());
	probe.depth->MakeWritable(cmdBuffer);

	cmdBuffer.BeginRenderPass(*renderpass, *probe.framebuffer, SubpassContents::Inline);
	timer->RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::TopOfPipe);
	cmdBuffer.BindGraphicsPipeline(*gfx);
	cmdBuffer.BindGraphicsDescriptor(*gfx, *probe.block);
	cmdBuffer.SetViewportAndScissor(probe.GetViewport());
}

void Pu::LightProbeRenderer::Render(CommandBuffer & cmdBuffer, const Model & model, const Matrix & transform)
{
	cmdBuffer.PushConstants(*gfx, ShaderStageFlags::Vertex, 0, sizeof(Matrix), &transform);

	const MeshCollection &meshes = model.GetMeshes();
	uint32 oldMatIdx = MeshCollection::DefaultMaterialIdx;
	uint32 oldVrtxView = Mesh::DefaultViewIdx;
	uint32 oldIdxView = Mesh::DefaultViewIdx;

	/* Try to render all the individual meshes. */
	for (const auto &[matIdx, mesh] : meshes)
	{
		if (mesh.GetStride() != sizeof(Advanced3D)) continue;
		if (matIdx == MeshCollection::DefaultMaterialIdx) continue;

		/* Update the bound material if needed. */
		if (matIdx != oldMatIdx && matIdx)
		{
			oldMatIdx = matIdx;
			cmdBuffer.BindGraphicsDescriptor(*gfx, model.GetProbeMaterial(matIdx));
		}

		/* Update the vertex binding if needed. */
		if (mesh.GetVertexView() != oldVrtxView)
		{
			oldVrtxView = mesh.GetVertexView();
			cmdBuffer.BindVertexBuffer(0, meshes.GetBuffer(), meshes.GetViewOffset(oldVrtxView));
		}

		/* Update the index binding if needed. */
		if (mesh.GetIndexView() != oldIdxView)
		{
			oldIdxView = mesh.GetIndexView();
			cmdBuffer.BindIndexBuffer(mesh.GetIndexType(), meshes.GetBuffer(), meshes.GetViewOffset(oldIdxView));
		}

		mesh.Draw(cmdBuffer, 0, 1);
	}
}

void Pu::LightProbeRenderer::End(LightProbe & probe, CommandBuffer & cmdBuffer) const
{
	if (probe.cycleMode == LightProbe::CycleMode::Baked) return;

	/* We need the shader read access to use it as an environment map. */
	timer->RecordTimestamp(cmdBuffer, 0, PipelineStageFlags::BottomOfPipe);
	cmdBuffer.EndRenderPass();
	cmdBuffer.MemoryBarrier(*probe.image, PipelineStageFlags::ColorAttachmentOutput, PipelineStageFlags::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlags::ShaderRead, probe.texture->GetFullRange());
	cmdBuffer.EndLabel();
	Profiler::End();
	probe.Unlock();
}

Pu::DescriptorPool * Pu::LightProbeRenderer::CreateDescriptorPool(uint32 maxMaterials) const
{
	if (!renderpass->IsLoaded())
	{
		Log::Error("Unable to create descriptor pool for light probe renderer when renderpass is not yet loaded!");
		return nullptr;
	}

	/* Set 1 is the material set. */
	return new DescriptorPool(*renderpass, maxMaterials, 0, 1);
}

void Pu::LightProbeRenderer::InitializeRenderpass(Renderpass &)
{
	Subpass &subpass = renderpass->GetSubpass(0);

	/* The shader read only layout is used when the light probe is used as an environment, but we need it to be a color attachment for 1 subpass. */
	Output &output = subpass.GetOutput("Color");
	output.SetFormat(Format::R8G8B8A8_SRGB);
	output.SetInitialLayout(ImageLayout::ShaderReadOnlyOptimal);
	output.SetLayout(ImageLayout::ColorAttachmentOptimal);
	output.SetFinalLayout(ImageLayout::ShaderReadOnlyOptimal);

	/* The light probes use D16 depth format for their depth buffers. */
	subpass.AddDepthStencil().SetDepthDescription(Format::D16_UNORM);

	/* We only handle the texture coordinate in this shader. */
	subpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Advanced3D, TexCoord));
}

void Pu::LightProbeRenderer::InitializePipeline(Renderpass &)
{
	/* We need to set the viewport for every individual light probe so that has to be a dynamic state. */
	gfx = new GraphicsPipeline(*renderpass, 0);
	gfx->SetTopology(PrimitiveTopology::TriangleList);
	gfx->AddVertexBinding<Advanced3D>(0);
	gfx->EnableDepthTest(true, CompareOp::LessOrEqual);
	gfx->AddDynamicState(DynamicState::ViewPort);
	gfx->AddDynamicState(DynamicState::Scissor);
	gfx->Finalize();

	/* This descriptor pool is for the view transformations. */
	pool = new DescriptorPool(*renderpass, maxSets, 0, 0);
}

void Pu::LightProbeRenderer::Destroy(void)
{
	if (pool) delete pool;
	if (gfx) delete gfx;
	if (renderpass) loader->Release(*renderpass);
	if (timer) delete timer;
}