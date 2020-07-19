#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/VertexLayouts/Advanced3D.h"
#include "Graphics/Textures/TextureInput2D.h"
#include "Graphics/Models/ShapeCreator.h"
#include "Graphics/VertexLayouts/PointLight.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"

using namespace Pu;

#ifdef _DEBUG
#include "Graphics/Diagnostics/QueryChain.h"
#include "Core/Diagnostics/Profiler.h"
#include <imgui/include/imgui.h>

#define DBG_CHECK_SUBPASS(required)			if (static_cast<int32>(required) != activeSubpass) Log::Fatal("Invalid function call, subpass %u expected, but subpass %d is active!", required, activeSubpass)
#else
#define DBG_CHECK_SUBPASS(...)
#endif

constexpr uint32 TerrainTimer = 0;
constexpr uint32 GeometryTimer = 1;
constexpr uint32 SkyboxTimer = 2;
constexpr uint32 LightingTimer = 3;
constexpr uint32 PostTimer = 4;

constexpr int32 SubpassNone = -2;
constexpr int32 SubpassInitialized = -1;
constexpr uint32 SubpassBeginFinal = DeferredRenderer::SubpassPointLight;

constexpr inline bool is_geometry_subpass(uint32 subpass)
{
	return subpass == DeferredRenderer::SubpassBasicStaticGeometry
		|| subpass == DeferredRenderer::SubpassAdvancedStaticGeometry
		|| subpass == DeferredRenderer::SubpassBasicMorphGeometry;
}

constexpr inline bool is_lighting_subpass(uint32 subpass)
{
	return subpass == DeferredRenderer::SubpassDirectionalLight
		|| subpass == DeferredRenderer::SubpassPointLight;
}

class LightVolumeStageTask
	: public Task
{
public:
	LightVolumeStageTask(AssetFetcher &fetcher, MeshCollection &result)
		: result(result), fetcher(fetcher)
	{}

	Result Execute(void) final
	{
		StagingBuffer *staging = new StagingBuffer(fetcher.GetDevice(), ShapeCreator::GetVolumeSphereBufferSize(EllipsiodDivs));
		Mesh mesh = ShapeCreator::VolumeSphere(*staging, EllipsiodDivs);
		result.Initialize(fetcher.GetDevice(), *staging, ShapeCreator::GetVolumeSphereVertexSize(EllipsiodDivs), std::move(mesh));

		fetcher.GetLoader().StageBuffer(*staging, result.GetBuffer(), PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
		return Result::AutoDelete();
	}

private:
	AssetFetcher &fetcher;
	MeshCollection &result;
};

/*
	The shaders define the following descriptor sets:
	0: Camera (All subpasses)
		Projection
		View
		Frustum
		Window Size
		Inverse Projection
		Inverse View
		Position
		Exposure
		Brightness
		Contrast
	1: Terrain (Terrain subpass)
		Model Matrix
		Layer Ranges
		Displacement Factor
		Tessellation Factor
		Edge Size
		Height Map
		Diffuse Maps
	1: Material (Static and advanced)
		Diffuse
		Specular / Glossiness
		Bump
		Emissive
		Occlusion
		F0 / Specular Power
		Diffuse Factor / Roughness
		Alpha Threshold
	1: Input Attachments (Light subpasses)
		Diffuse / Roughness
		Specular / Specular power
		Normal
		Emissive / AO
		HDR buffer
	2: Directional Light (Directional light subpass)
		Direction
		Radiance (pre-multiplied)
		Environment map (temporary?)

	The framebuffer has several attachments:			G-Pass		Light-Pass		Sky-Pass	Post-Pass		Default Idx
	0: Swapchain				[r, g, b, a]			-			-				-			Color			5
	1: G-Buffer (Diffuse)		[r, g, b, r]			Color		Input			-			-				0
	2: G-Buffer (Specular)		[r, g, b, power]		Color		Input			-			-				1
	3: G-Buffer (Normal)		[x, y, z, w]			Color		Input			-			-				2
	4: HDR-Buffer				[r, g, b, a]			-			Color			Color		Input			0
	5: G-Buffer (Depth)			[d]						Depth		Input			Depth		-				3 & 1

	We need to override the attachment reference in most of the subpasses.

	G-Buffer attachments should be kept small, <= 128 bytes per pixel.
	This is so the GPU can use tiled memory.
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, GameWindow & wnd, bool wireframe)
	: wnd(&wnd), depthBuffer(nullptr), markNeeded(true), fetcher(&fetcher), skybox(nullptr),
	gfxTerrain(nullptr), gfxGPassBasic(nullptr), gfxGPassAdv(nullptr), gfxDLight(nullptr),
	gfxPLight(nullptr), gfxSkybox(nullptr), gfxTonePass(nullptr), curCmd(nullptr), curCam(nullptr),
	wireframe(wireframe), descPoolInput(nullptr), descSetInput(nullptr), advanced(false), 
	renderpassStarted(false), activeSubpass(SubpassNone), lightVolumes(new MeshCollection())
{
#ifdef _DEBUG
	QueryPipelineStatisticFlag statFlags = QueryPipelineStatisticFlag::VertexShaderInvocations | QueryPipelineStatisticFlag::FragmentShaderInvocations;
#endif

	/* Make sure that we can actually run wireframe mode. */
	if (wireframe)
	{
		if (!wnd.GetDevice().GetPhysicalDevice().GetEnabledFeatures().FillModeNonSolid)
		{
			Log::Warning("Cannot use DeferredRenderer in wireframe mode (FillModeNonSolid was not enabled)!");
			wireframe = false;
		}
	}

	/* We can only use tessellation on the terrain if it's supported. */
	vector<wstring> terrainShaders;
	if (wnd.GetDevice().GetPhysicalDevice().GetEnabledFeatures().TessellationShader)
	{
		terrainShaders = { L"{Shaders}PatchTerrain.vert.spv", L"{Shaders}Terrain.tesc.spv", L"{Shaders}Terrain.tese.spv", L"{Shaders}Terrain.frag.spv" };
#ifdef _DEBUG
		statFlags |= QueryPipelineStatisticFlag::TessellationEvaluationShaderInvocations;
#endif
	}
	else
	{
		terrainShaders = { L"{Shaders}FastTerrain.vert.spv", L"{Shaders}Terrain.frag.spv" };
	}

	renderpass = &fetcher.FetchRenderpass(
		{
			terrainShaders,
			{ L"{Shaders}BasicStaticGeometry.vert.spv", L"{Shaders}BasicGeometry.frag.spv" },
			{ L"{Shaders}AdvancedStaticGeometry.vert.spv", L"{Shaders}AdvancedGeometry.frag.spv" },
			{ L"{Shaders}BasicMorphGeometry.vert.spv", L"{Shaders}BasicGeometry.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}DirectionalLight.frag.spv" },
			{ L"{Shaders}WorldLight.vert.spv", L"{Shaders}PointLight.frag.spv" },
			{ L"{Shaders}Skybox.vert.spv", L"{Shaders}Skybox.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}CameraEffects.frag.spv" }
		});

	/* Create the light volumes. */
	Task *task = new LightVolumeStageTask(fetcher, *lightVolumes);
	fetcher.GetScheduler().Spawn(*task);

	/* We need to recreate resources if the window resizes, or the color space is changed. */
	wnd.SwapchainRecreated.Add(*this, &DeferredRenderer::OnSwapchainRecreated);
	renderpass->PreCreate.Add(*this, &DeferredRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DeferredRenderer::FinalizeRenderpass);
	CreateSizeDependentResources();

#ifdef _DEBUG
	timer = new QueryChain(wnd.GetDevice(), QueryType::Timestamp, 5);
	stats = new QueryChain(wnd.GetDevice(), statFlags);
#endif
}

Pu::DescriptorPool * Pu::DeferredRenderer::CreateTerrainDescriptorPool(uint32 maxTerrains) const
{
	if (!renderpass->IsLoaded())
	{
		Log::Error("Unable to create descriptor pool from deferred renderer when renderpas is not yet loaded!");
		return nullptr;
	}

	/* Set 1 is the terrain set. */
	return new DescriptorPool(*renderpass, maxTerrains, SubpassTerrain, 1);
}

Pu::DescriptorPool * Pu::DeferredRenderer::CreateMaterialDescriptorPool(uint32 maxBasicMaterials, uint32 maxAdvancedMaterials) const
{
	if (!renderpass->IsLoaded())
	{
		Log::Error("Unable to create descriptor pool from deferred renderer when renderpas is not yet loaded!");
		return nullptr;
	}

	/* Allocate the pool, only add the set if the user wants to use it. */
	DescriptorPool *result = new DescriptorPool(*renderpass);
	if (maxBasicMaterials) result->AddSet(SubpassBasicStaticGeometry, 1, maxBasicMaterials);
	if (maxAdvancedMaterials) result->AddSet(SubpassAdvancedStaticGeometry, 1, maxAdvancedMaterials);
	return result;
}

void Pu::DeferredRenderer::InitializeCameraPool(DescriptorPool & pool, uint32 maxSets) const
{
	pool.AddSet(SubpassTerrain, 0, maxSets);
	pool.AddSet(SubpassAdvancedStaticGeometry, 0, maxSets);
	pool.AddSet(SubpassDirectionalLight, 0, maxSets);
	pool.AddSet(SubpassPointLight, 0, maxSets);				
	pool.AddSet(SubpassSkybox, 0, maxSets);
	pool.AddSet(SubpassPostProcessing, 0, maxSets);
}

void Pu::DeferredRenderer::InitializeResources(CommandBuffer & cmdBuffer, const Camera & camera)
{
#ifdef _DEBUG
	/* Update the profiler and reset the queries. */
	Profiler::Begin("Rendering", Color::Red());
	Profiler::Add("Rendering (Environment)", Color::Gray(), timer->GetProfilerTimeDelta(TerrainTimer) + timer->GetProfilerTimeDelta(SkyboxTimer));
	Profiler::Add("Rendering (Geometry)", Color::Red(), timer->GetProfilerTimeDelta(GeometryTimer));
	Profiler::Add("Rendering (Lighting)", Color::Yellow(), timer->GetProfilerTimeDelta(LightingTimer));
	Profiler::Add("Rendering (Post-Processing)", Color::Green(), timer->GetProfilerTimeDelta(PostTimer));

	timer->Reset(cmdBuffer);
	stats->Reset(cmdBuffer);
#endif

	/* Make sure we only do this if needed. */
	curCmd = &cmdBuffer;
	curCam = &camera;
	activeSubpass = SubpassInitialized;

	if (markNeeded)
	{
		markNeeded = false;

		/* Mark all the framebuffer images as writable. */
		depthBuffer->MakeWritable(cmdBuffer);
		for (const TextureInput2D *attachment : textures)
		{
			cmdBuffer.MemoryBarrier(*attachment, PipelineStageFlag::TopOfPipe, PipelineStageFlag::ColorAttachmentOutput, ImageLayout::ColorAttachmentOptimal, AccessFlag::ColorAttachmentWrite, attachment->GetFullRange(), DependencyFlag::ByRegion);
		}
	}
}

void Pu::DeferredRenderer::Begin(uint32 subpass)
{
	/* Early out if this subpass is already active. */
	const uint32 uActiveSubpass = static_cast<uint32>(activeSubpass);
	if (uActiveSubpass == subpass) return;

	/* Check if the user is calling in the correct order on debug mode. */
#ifdef _DEBUG
	if (activeSubpass == SubpassNone) Log::Fatal("InitializeResources should be called before any Begin subpass call!");
	if (static_cast<int32>(subpass) < activeSubpass) Log::Fatal("Cannot begin subpass %u after subpass %d!", subpass, activeSubpass);
	if (subpass > SubpassBeginFinal) Log::Fatal("Invalid subpass passed!");
#endif

	/* Finalize the old subpass. */
	EndSubpass(subpass, uActiveSubpass);

	/* Add a debug label and record a timestamp on debug mode. */
#ifdef _DEBUG
	string label = "Deferred Renderer (";
	switch (subpass)
	{
	case SubpassTerrain:
		label += "Terrain";
		timer->RecordTimestamp(*curCmd, TerrainTimer, PipelineStageFlag::TopOfPipe);
		break;
	case SubpassBasicStaticGeometry:
		label += "Basic Static";
		break;
	case SubpassAdvancedStaticGeometry:
		label += "Advanced Static";
		break;
	case SubpassBasicMorphGeometry:
		label += "Basic Morph";
		break;
	case SubpassDirectionalLight:
		label += "Directional Light";
		break;
	case SubpassPointLight:
		label += "Point Light";
		break;
	default:
		Log::Fatal("This should never occur!");
		return;
	}

	label += ')';
	curCmd->AddLabel(label, Color::Blue());

	/* Start the geometry timer if this is the first geometry pass. */
	if (is_geometry_subpass(subpass) && !is_geometry_subpass(uActiveSubpass))
	{
		timer->RecordTimestamp(*curCmd, GeometryTimer, PipelineStageFlag::TopOfPipe);
	}

	/* Start the lighting timer if this is the first ligh pass. */
	if (is_lighting_subpass(subpass) && !is_lighting_subpass(uActiveSubpass))
	{
		timer->RecordTimestamp(*curCmd, LightingTimer, PipelineStageFlag::TopOfPipe);
	}
#endif

	/* 
	Select the correct graphics pipeline to bind.
	And bind any additional subpass global graphics descriptors.
	*/
	const GraphicsPipeline *curGfx;
	switch (subpass)
	{
	case SubpassTerrain:
		curGfx = gfxTerrain;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassTerrain, *curCam);
		break;
	case SubpassBasicStaticGeometry:
		curGfx = gfxGPassBasic;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassAdvancedStaticGeometry, *curCam);
		break;
	case SubpassAdvancedStaticGeometry:
		curGfx = gfxGPassAdv;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassAdvancedStaticGeometry, *curCam);
		break;
	case SubpassBasicMorphGeometry:
		curGfx = gfxGPassBasicMorph;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassAdvancedStaticGeometry, *curCam);
		break;
	case SubpassDirectionalLight:
		curGfx = gfxDLight;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassDirectionalLight, *descSetInput);
		curCmd->BindGraphicsDescriptors(*curGfx, subpass, *curCam);
		break;
	case SubpassPointLight:
		curGfx = gfxPLight;
		curCmd->BindGraphicsDescriptors(*curGfx, SubpassDirectionalLight, *descSetInput);
		curCmd->BindGraphicsDescriptors(*curGfx, subpass, *curCam);
		break;
	default:
		Log::Fatal("This should never occur!");
		return;
	}

	if (subpass == SubpassPointLight)
	{
		/* Bind the point light volume sphere to the first binding. */
		const Mesh &sphere = lightVolumes->GetShape(0).second;
		curCmd->BindVertexBuffer(0, lightVolumes->GetBuffer(), lightVolumes->GetViewOffset(sphere.GetVertexView()));
		curCmd->BindIndexBuffer(sphere.GetIndexType(), lightVolumes->GetBuffer(), lightVolumes->GetViewOffset(sphere.GetIndexView()));
	}

	/* A graphics pipeline should always be bound. */
	curCmd->BindGraphicsPipeline(*curGfx);
	activeSubpass = static_cast<int32>(subpass);
}

void Pu::DeferredRenderer::End(void)
{
	/* We should ignore this call if nothing was rendered. */
	if (activeSubpass == SubpassNone) return;
	const uint32 uActiveSubpass = static_cast<uint32>(activeSubpass);

	/* End the current subpass on debug mode. */
	EndSubpass(skybox ? SubpassSkybox : SubpassPostProcessing, activeSubpass);

	/* Attempt to do a skybox pass and do the post-processing pass. */
	DoSkybox();
	DoTonemap();

#ifdef _DEBUG
	stats->RecordStatistics(*curCmd);
#endif

	curCmd->EndRenderPass();

	/* Setting these to nullptr gives us the option to catch invalid usage. */
	curCmd = nullptr;
	curCmd = nullptr;
	renderpassStarted = false;
	activeSubpass = SubpassNone;

#ifdef _DEBUG
	Profiler::End();
#endif
}

void Pu::DeferredRenderer::Render(const TerrainChunk & chunk)
{
	DBG_CHECK_SUBPASS(SubpassTerrain);
	if (curCam->Cull(chunk.GetBoundingBox() + chunk.GetPosition())) return;

	const MeshCollection &meshes = chunk.GetMeshes();
	curCmd->BindGraphicsDescriptor(*gfxTerrain, chunk.GetMaterial());

	for (const auto &[_, mesh] : chunk.GetMeshes())
	{
		curCmd->BindVertexBuffer(0, meshes.GetBuffer(), meshes.GetViewOffset(mesh.GetVertexView()));
		curCmd->BindIndexBuffer(mesh.GetIndexType(), meshes.GetBuffer(), meshes.GetViewOffset(mesh.GetIndexView()));
		mesh.Draw(*curCmd, 1);
	}
}

void Pu::DeferredRenderer::Render(const Model & model, const Matrix & transform)
{
	/* Only render the model if it can be viewed by the camera. */
	const MeshCollection &meshes = model.GetMeshes();
	if (curCam->Cull(meshes.GetBoundingBox(), transform)) return;

	/* Set the model matrix. */
	const GraphicsPipeline &pipeline = *(advanced ? gfxGPassAdv : gfxGPassBasic);
	curCmd->PushConstants(pipeline, ShaderStageFlag::Vertex, 0, sizeof(Matrix), transform.GetComponents());

	const uint32 requiredStride = pipeline.GetVertexStride(0);
	uint32 oldMatIdx = MeshCollection::DefaultMaterialIdx;
	uint32 oldVrtxView = Mesh::DefaultViewIdx;
	uint32 oldIdxView = Mesh::DefaultViewIdx;

	/* Try to render all the individual meshes. */
	for (const auto &[matIdx, mesh] : meshes)
	{
		/* Cull the mesh if any of the following conditions are met. */
		if (matIdx == MeshCollection::DefaultMaterialIdx) continue;
		if (mesh.GetStride() != requiredStride) continue;
		if (curCam->Cull(mesh.GetBoundingBox(), transform)) continue;

		/* Update the bound material if needed. */
		if (matIdx != oldMatIdx)
		{
			oldMatIdx = matIdx;
			curCmd->BindGraphicsDescriptor(pipeline, model.GetMaterial(matIdx));
		}

		/* Update the vertex binding if needed. */
		if (mesh.GetVertexView() != oldVrtxView)
		{
			oldVrtxView = mesh.GetVertexView();
			curCmd->BindVertexBuffer(0, meshes.GetBuffer(), meshes.GetViewOffset(oldVrtxView));
		}

		/* Update the index binding if needed. */
		if (mesh.GetIndexView() != oldIdxView)
		{
			oldIdxView = mesh.GetIndexView();
			curCmd->BindIndexBuffer(mesh.GetIndexType(), meshes.GetBuffer(), meshes.GetViewOffset(oldIdxView));
		}

		/* Render the mesh. */
		mesh.Draw(*curCmd, 1);
	}
}

void Pu::DeferredRenderer::Render(const Model & model, const Matrix & transform, uint32 keyFrame1, uint32 keyFrame2, float blending)
{
	DBG_CHECK_SUBPASS(SubpassBasicMorphGeometry);

	/* Only render the model if it can be viewed by the camera. */
	const MeshCollection &meshes = model.GetMeshes();
	const AABB bb = lerp(meshes.GetBoundingBox(keyFrame1), meshes.GetBoundingBox(keyFrame2), blending);
	if (curCam->Cull(bb, transform)) return;

	/* Set push constants. */
	float push[17];
	memcpy(push, transform.GetComponents(), sizeof(Matrix));
	push[16] = blending;
	curCmd->PushConstants(*gfxGPassBasicMorph, ShaderStageFlag::Vertex, 0, sizeof(push), push);

	/* Query the two meshes from the model. */
	const auto &[matIdx1, mesh1] = meshes.GetShape(keyFrame1);
	const auto &[matIdx2, mesh2] = meshes.GetShape(keyFrame2);

	/* Do a quick check on debug, to see if the meshes are compatible. */
#ifdef _DEBUG
	if (matIdx1 != matIdx2) Log::Fatal("Cannot use keyframe %u and %u as morph targets (material mismatch)!", keyFrame1, keyFrame2);
	if (mesh1.GetCount() != mesh2.GetCount()) Log::Fatal("Cannot use keyframe %u and %u as morph targets (meshes don't have equal vertex count)!", keyFrame1, keyFrame2);
#endif

	/* Bind the required buffers and descriptor and render the meshes. */
	curCmd->BindGraphicsDescriptor(*gfxGPassBasicMorph, model.GetMaterial(matIdx1));
	meshes.Bind(*curCmd, 0, keyFrame1);
	meshes.Bind(*curCmd, 1, keyFrame2);
	curCmd->Draw(mesh1.GetCount(), 1, 0, 0);
}

void Pu::DeferredRenderer::Render(const DirectionalLight & light)
{
	DBG_CHECK_SUBPASS(SubpassDirectionalLight);
	curCmd->BindGraphicsDescriptor(*gfxDLight, light);
	curCmd->Draw(3, 1, 0, 0);
}

void Pu::DeferredRenderer::Render(const PointLightPool & lights)
{
	DBG_CHECK_SUBPASS(SubpassPointLight);
	curCmd->BindVertexBuffer(1, lights, 0);
	lightVolumes->GetShape(0).second.Draw(*curCmd, lights.GetLightCount());
}

void Pu::DeferredRenderer::SetSkybox(const TextureCube & texture)
{
	if (descSetInput)
	{
		skybox = &renderpass->GetSubpass(SubpassSkybox).GetDescriptor("Skybox");
		descSetInput->Write(SubpassSkybox, *skybox, texture);
	}
	else Log::Fatal("Cannot set skybox when deferred rendering is not yet finalized!");
}

void Pu::DeferredRenderer::Visualize(void) const
{
#ifdef _DEBUG
	if constexpr (ImGuiAvailable)
	{
		const vector<uint32> results = stats->GetStatistics();
		if (results.size())
		{
			if (ImGui::Begin("Pipeline Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				for (uint32 i = 0; i < results.size(); i++)
				{
					ImGui::Text("%s: %u", stats->GetStatisticName(i), results[i]);
				}

				ImGui::End();
			}
		}
	}
#endif
}

void Pu::DeferredRenderer::EndSubpass(uint32 newSubpass, uint32 uActiveSubpass)
{
#ifndef _DEBUG
	(void)uActiveSubpass;
#endif

	if (!renderpassStarted)
	{
		/* Begin the renderpass and begin the pipeline statistics recording. */
		renderpassStarted = true;
		curCmd->BeginRenderPass(*renderpass, wnd->GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);

#ifdef _DEBUG
		stats->RecordStatistics(*curCmd);
#endif
	}
#ifdef _DEBUG
	else
	{
		/* End the previous subpass. */
		curCmd->EndLabel();

		/* End the terrain timer if needed. */
		if (uActiveSubpass == SubpassTerrain)
		{
			timer->RecordTimestamp(*curCmd, TerrainTimer, PipelineStageFlag::BottomOfPipe);
		}

		/* End the geometry timer if needed. */
		if (is_geometry_subpass(uActiveSubpass) && !is_geometry_subpass(newSubpass))
		{
			timer->RecordTimestamp(*curCmd, GeometryTimer, PipelineStageFlag::BottomOfPipe);
		}

		/* End the lighting timer if needed. */
		if (is_lighting_subpass(uActiveSubpass) && !is_lighting_subpass(newSubpass))
		{
			timer->RecordTimestamp(*curCmd, LightingTimer, PipelineStageFlag::BottomOfPipe);
		}
	}
#endif

	/* Skip to the correct subpass. */
	for (uint32 i = 0; i < newSubpass - max(0, activeSubpass); i++)
	{
		curCmd->NextSubpass(SubpassContents::Inline);
	}
}

void Pu::DeferredRenderer::DoSkybox(void)
{
	/* Skip rendering the skybox if none was set. */
	if (skybox)
	{
		curCmd->AddLabel("Deferred Renderer (Skybox)", Color::Blue());
#ifdef _DEBUG
		timer->RecordTimestamp(*curCmd, SkyboxTimer, PipelineStageFlag::TopOfPipe);
#endif
		curCmd->BindGraphicsPipeline(*gfxSkybox);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, SubpassSkybox, *curCam);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, SubpassSkybox, *descSetInput);
		curCmd->Draw(3, 1, 0, 0);
		curCmd->EndLabel();
#ifdef _DEBUG
		timer->RecordTimestamp(*curCmd, SkyboxTimer, PipelineStageFlag::BottomOfPipe);
#endif
	}
	else curCmd->NextSubpass(SubpassContents::Inline);
}

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Camera Effects)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
#ifdef _DEBUG
	timer->RecordTimestamp(*curCmd, PostTimer, PipelineStageFlag::TopOfPipe);
#endif
	curCmd->BindGraphicsPipeline(*gfxTonePass);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, SubpassPostProcessing, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, SubpassPostProcessing, *descSetInput);
	curCmd->Draw(3, 1, 0, 0);
	curCmd->EndLabel();
#ifdef _DEBUG
	timer->RecordTimestamp(*curCmd, PostTimer, PipelineStageFlag::BottomOfPipe);
#endif
}

void Pu::DeferredRenderer::OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs & args)
{
	/*
	We can't ignore the event fully if the renderpass isn't loaded yet.
	The size dependent resources will always need to be recreated,
	but the renderpass and pipeline don't have to change.
	*/
	if (renderpass->IsLoaded())
	{
		/*
		We need to recreate the entire renderpass if the format changed,
		but we only have to recreate the framebuffers and pipelines if the area changed.
		*/
		if (args.FormatChanged) renderpass->Recreate();
		if (args.AreaChanged)
		{
			CreateSizeDependentResources();
			FinalizeRenderpass(*renderpass);
		}
	}
	else CreateSizeDependentResources();
}

void Pu::DeferredRenderer::InitializeRenderpass(Renderpass &)
{
	/* Set all the options for the terrain Geometry-Pass. */
	{
		Subpass &tpass = renderpass->GetSubpass(SubpassTerrain);

		Output &depth = tpass.AddDepthStencil();
		depth.SetFormat(depthBuffer->GetFormat());
		depth.SetClearValue({ 1.0f, 0 });
		depth.SetLayouts(ImageLayout::DepthStencilAttachmentOptimal);
		depth.SetReference(5);

		Output &diffA2 = tpass.GetOutput("GBufferDiffuseRough");
		diffA2.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		diffA2.SetFormat(textures[0]->GetImage().GetFormat());
		diffA2.SetStoreOperation(AttachmentStoreOp::DontCare);
		diffA2.SetReference(1);

		Output &spec = tpass.GetOutput("GBufferSpecular");
		spec.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		spec.SetFormat(textures[1]->GetImage().GetFormat());
		spec.SetStoreOperation(AttachmentStoreOp::DontCare);
		spec.SetReference(2);

		Output &norm = tpass.GetOutput("GBufferNormal");
		norm.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		norm.SetFormat(textures[2]->GetImage().GetFormat());
		norm.SetStoreOperation(AttachmentStoreOp::DontCare);
		norm.SetReference(3);

		tpass.GetAttribute("Normal").SetOffset(vkoffsetof(Patched3D, Normal));
		tpass.GetAttribute("TexCoord1").SetOffset(vkoffsetof(Patched3D, TexCoord1));
		tpass.GetAttribute("TexCoord2").SetOffset(vkoffsetof(Patched3D, TexCoord2));
	}

	/* Set all the options for the basic static Geometry-Pass. */
	{
		Subpass &gpass = renderpass->GetSubpass(SubpassBasicStaticGeometry);
		gpass.SetNoDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, DependencyFlag::ByRegion);

		/* We can just clone the values set by the previous subpass. */
		gpass.CloneDepthStencil(5);
		gpass.CloneOutput("GBufferDiffuseRough", 1);
		gpass.CloneOutput("GBufferSpecular", 2);
		gpass.CloneOutput("GBufferNormal", 3);

		gpass.GetAttribute("Normal").SetOffset(vkoffsetof(Basic3D, Normal));
		gpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Basic3D, TexCoord));
	}

	/* Set all the options for the advanced static Geometry-Pass. */
	{
		Subpass &gpass = renderpass->GetSubpass(SubpassAdvancedStaticGeometry);
		gpass.SetNoDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, DependencyFlag::ByRegion);

		/* We can just clone the values set by the previous subpass. */
		gpass.CloneDepthStencil(5);
		gpass.CloneOutput("GBufferDiffuseRough", 1);
		gpass.CloneOutput("GBufferSpecular", 2);
		gpass.CloneOutput("GBufferNormal", 3);

		gpass.GetAttribute("Normal").SetOffset(vkoffsetof(Advanced3D, Normal));
		gpass.GetAttribute("Tangent").SetOffset(vkoffsetof(Advanced3D, Tangent));
		gpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Advanced3D, TexCoord));
	}

	/* Set all the options for the basic morph Geometry-Pass. */
	{
		Subpass &gpass = renderpass->GetSubpass(SubpassBasicMorphGeometry);
		gpass.SetNoDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, DependencyFlag::ByRegion);

		/* We can just clone the values set by the previous subpass. */
		gpass.CloneDepthStencil(5);
		gpass.CloneOutput("GBufferDiffuseRough", 1);
		gpass.CloneOutput("GBufferSpecular", 2);
		gpass.CloneOutput("GBufferNormal", 3);

		gpass.GetAttribute("Normal1").SetOffset(vkoffsetof(Basic3D, Normal));
		gpass.GetAttribute("TexCoord1").SetOffset(vkoffsetof(Basic3D, TexCoord));

		Attribute &pos2 = gpass.GetAttribute("Position2");
		pos2.SetBinding(1);

		Attribute &normal2 = gpass.GetAttribute("Normal2");
		normal2.SetBinding(1);
		normal2.SetOffset(vkoffsetof(Basic3D, Normal));

		Attribute &uv2 = gpass.GetAttribute("TexCoord2");
		uv2.SetBinding(1);
		uv2.SetOffset(vkoffsetof(Basic3D, TexCoord));
	}

	/* Set all the options for the directional light pass. */
	{
		Subpass &dlpass = renderpass->GetSubpass(SubpassDirectionalLight);

		/* The depth buffer needs to be moved from a Write Attachment to an Input Attachment. */
		dlpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &hdr = dlpass.GetOutput("L0");
		hdr.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[3]->GetImage().GetFormat());
		hdr.SetLoadOperation(AttachmentLoadOp::DontCare);
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(4);
	}

	/* Set all the options for the point light pass. */
	{
		Subpass &plpass = renderpass->GetSubpass(SubpassPointLight);
		plpass.SetNoDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, DependencyFlag::ByRegion);

		plpass.CloneOutput("L0", 4);

		Attribute &instAtt = plpass.GetAttribute("InstanceAttenuation");
		Attribute &instRad = plpass.GetAttribute("InstanceRadiance");
		Attribute &instVol = plpass.GetAttribute("InstanceVolume");

		instAtt.SetBinding(1);
		instRad.SetBinding(1);
		instVol.SetBinding(1);

		instRad.SetOffset(vkoffsetof(PointLight, Radiance));
		instAtt.SetOffset(vkoffsetof(PointLight, AttenuationC));
	}

	/* Set all the options for the skybox pass. */
	{
		Subpass &skypass = renderpass->GetSubpass(SubpassSkybox);
		skypass.SetNoDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, DependencyFlag::ByRegion);

		skypass.CloneDepthStencil(5);

		Output &hdr = skypass.GetOutput("Hdr");
		hdr.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[3]->GetImage().GetFormat());
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(4);
	}

	/* Set all the options for the camera pass. */
	{
		Subpass &fpass = renderpass->GetSubpass(SubpassPostProcessing);
		fpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &screen = fpass.GetOutput("FragColor");
		screen.SetDescription(wnd->GetSwapchain());
		screen.SetLoadOperation(AttachmentLoadOp::DontCare);
		screen.SetReference(0);
	}

	/* Make the swapchain image ready for the OS to present it. */
	renderpass->AddDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::BottomOfPipe, AccessFlag::ColorAttachmentWrite, AccessFlag::MemoryRead, DependencyFlag::ByRegion);
}

void Pu::DeferredRenderer::FinalizeRenderpass(Renderpass &)
{
	/* We need to allocate the pipelines on the first renderpass finalize call. */
	if (!gfxTonePass)
	{
		gfxTerrain = new GraphicsPipeline(*renderpass, SubpassTerrain);
		gfxGPassBasic = new GraphicsPipeline(*renderpass, SubpassBasicStaticGeometry);
		gfxGPassAdv = new GraphicsPipeline(*renderpass, SubpassAdvancedStaticGeometry);
		gfxGPassBasicMorph = new GraphicsPipeline(*renderpass, SubpassBasicMorphGeometry);
		gfxDLight = new GraphicsPipeline(*renderpass, SubpassDirectionalLight);
		gfxPLight = new GraphicsPipeline(*renderpass, SubpassPointLight);
		gfxSkybox = new GraphicsPipeline(*renderpass, SubpassSkybox);
		gfxTonePass = new GraphicsPipeline(*renderpass, SubpassPostProcessing);

		/* We only need to create the descriptor set for the input attachments once. */
		descPoolInput = new DescriptorPool(*renderpass);
		descPoolInput->AddSet(SubpassDirectionalLight, 1, 1);
		descPoolInput->AddSet(SubpassSkybox, 1, 1);
		descPoolInput->AddSet(SubpassPostProcessing, 1, 1);

		descSetInput = new DescriptorSetGroup(*descPoolInput);
		descSetInput->Add(SubpassDirectionalLight, renderpass->GetSubpass(SubpassDirectionalLight).GetSetLayout(1));
		descSetInput->Add(SubpassSkybox, renderpass->GetSubpass(SubpassSkybox).GetSetLayout(1));
		descSetInput->Add(SubpassPostProcessing, renderpass->GetSubpass(SubpassPostProcessing).GetSetLayout(1));

		/* Write the input attachments to the descriptor set. */
		WriteDescriptors();
	}

	/* Create the graphics pipeline for the terrain pass. */
	{
		if (wireframe)
		{
			gfxTerrain->SetPolygonMode(PolygonMode::Line);
			gfxTerrain->SetLineWidth(2.0f);
		}

		if (wnd->GetDevice().GetPhysicalDevice().GetEnabledFeatures().TessellationShader)
		{
			gfxTerrain->SetTopology(PrimitiveTopology::PatchList);
			gfxTerrain->SetPatchControlPoints(4);
		}
		else
		{
			gfxTerrain->SetTopology(PrimitiveTopology::TriangleList);
		}

		gfxTerrain->SetViewport(wnd->GetNative().GetClientBounds());

		gfxTerrain->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxTerrain->SetCullMode(CullModeFlag::Back);
		gfxTerrain->AddVertexBinding<Patched3D>(0);
		gfxTerrain->Finalize();
		gfxTerrain->SetDebugName("Deferred Renderer Terrain");
	}

	/* Create the graphics pipeline for the basic static geometry pass. */
	{
		if (wireframe) gfxGPassBasic->SetPolygonMode(PolygonMode::Line);

		gfxGPassBasic->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPassBasic->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPassBasic->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPassBasic->AddVertexBinding<Basic3D>(0);
		gfxGPassBasic->Finalize();
		gfxGPassBasic->SetDebugName("Deferred Renderer Basic Static");
	}

	/* Create the graphics pipeline for the advanced static geometry pass. */
	{
		if (wireframe) gfxGPassAdv->SetPolygonMode(PolygonMode::Line);

		gfxGPassAdv->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPassAdv->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPassAdv->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPassAdv->AddVertexBinding<Advanced3D>(0);
		gfxGPassAdv->Finalize();
		gfxGPassAdv->SetDebugName("Deferred Renderer Advanced Static");
	}

	/* Create the graphics pipeline for the basic morph geometry pass. */
	{
		if (wireframe) gfxGPassBasicMorph->SetPolygonMode(PolygonMode::Line);

		gfxGPassBasicMorph->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPassBasicMorph->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPassBasicMorph->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPassBasicMorph->AddVertexBinding<Basic3D>(0);
		gfxGPassBasicMorph->AddVertexBinding<Basic3D>(1);
		gfxGPassBasicMorph->Finalize();
		gfxGPassBasicMorph->SetDebugName("Deferred Renderer Basic Morph");
	}

	/* Create the graphics pipeline for the directional light pass. */
	{
		gfxDLight->SetViewport(wnd->GetNative().GetClientBounds());
		gfxDLight->SetTopology(PrimitiveTopology::TriangleList);
		gfxDLight->GetBlendState("L0").SetAllBlendFactors(BlendFactor::One);
		gfxDLight->Finalize();
		gfxDLight->SetDebugName("Deferred Renderer Directional Light");
	}

	/* Create the graphics pipeline for the point light pass. */
	{
		gfxPLight->SetViewport(wnd->GetNative().GetClientBounds());
		gfxPLight->SetTopology(PrimitiveTopology::TriangleList);
		gfxPLight->SetCullMode(CullModeFlag::Front);
		gfxPLight->GetBlendState("L0").SetAllBlendFactors(BlendFactor::One);
		gfxPLight->AddVertexBinding<Vector3>(0);
		gfxPLight->AddVertexBinding<PointLight>(1, VertexInputRate::Instance);
		gfxPLight->Finalize();
		gfxPLight->SetDebugName("Deferred Renderer Point Light");
	}

	/* Create the graphics pipeline for the skybox pass. */
	{
		gfxSkybox->SetViewport(wnd->GetNative().GetClientBounds());
		gfxSkybox->SetTopology(PrimitiveTopology::TriangleList);
		gfxSkybox->EnableDepthTest(false, CompareOp::LessOrEqual);
		gfxSkybox->Finalize();
		gfxSkybox->SetDebugName("Deferred Renderer Skybox");
	}

	/* Create the graphics pipeline for the camera pass. */
	{
		gfxTonePass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxTonePass->SetTopology(PrimitiveTopology::TriangleList);
		gfxTonePass->Finalize();
		gfxTonePass->SetDebugName("Deferred Renderer Post-Processing");
	}

	CreateFramebuffer();
}

/*
The function is only called on two events:
- Before the initial renderpass create.
- After the window resizes.
*/
void Pu::DeferredRenderer::CreateSizeDependentResources(void)
{
	/* Make sure we can actually use the input attachments on this system. */
	if (wnd->GetDevice().GetPhysicalDevice().GetLimits().MaxDescriptorSetInputAttachments < 5)
	{
		Log::Fatal("Cannot run deferred renderer on this system (cannot use Vulkan input attachments)!");
	}

	/* Destroy the old resources if needed. */
	DestroyWindowDependentResources();
	LogicalDevice &device = wnd->GetDevice();
	const Extent3D size{ wnd->GetSize(), 1 };
	const ImageUsageFlag gbufferUsage = ImageUsageFlag::ColorAttachment | ImageUsageFlag::InputAttachment | ImageUsageFlag::TransientAttachment;
	markNeeded = true;

	/* Create the new images. */
	depthBuffer = new DepthBuffer(device, Format::D32_SFLOAT, wnd->GetSize());
	Image *gbuffAttach1 = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R8G8B8A8_UNORM, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));
	Image *gbuffAttach2 = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R8G8B8A8_UNORM, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));
	Image *gbuffAttach3 = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::A2R10G10B10_UNORM_PACK32, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));
	Image *tmpHdrAttach = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R16G16B16A16_SFLOAT, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));

	/* Create the new image views and samplers. */
	textures.emplace_back(new TextureInput2D(*gbuffAttach1));
	textures.emplace_back(new TextureInput2D(*gbuffAttach2));
	textures.emplace_back(new TextureInput2D(*gbuffAttach3));
	textures.emplace_back(new TextureInput2D(*tmpHdrAttach));

#ifdef _DEBUG
	gbuffAttach1->SetDebugName("G-Buffer Diffuse/Roughness");
	gbuffAttach2->SetDebugName("G-Buffer Specular/Power");
	gbuffAttach3->SetDebugName("G-Buffer World Normal");
	depthBuffer->SetDebugName("G-Buffer Depth");
	tmpHdrAttach->SetDebugName("Deferred HDR Buffer");
#endif

	/* The descriptors are getting new images, so we need to update them (but not on the first frame). */
	if (descPoolInput) WriteDescriptors();
}

void Pu::DeferredRenderer::WriteDescriptors(void)
{
	descSetInput->Write(SubpassDirectionalLight, renderpass->GetSubpass(SubpassDirectionalLight).GetDescriptor("GBufferDiffuseRough"), *textures[0]);
	descSetInput->Write(SubpassDirectionalLight, renderpass->GetSubpass(SubpassDirectionalLight).GetDescriptor("GBufferSpecular"), *textures[1]);
	descSetInput->Write(SubpassDirectionalLight, renderpass->GetSubpass(SubpassDirectionalLight).GetDescriptor("GBufferNormal"), *textures[2]);
	descSetInput->Write(SubpassDirectionalLight, renderpass->GetSubpass(SubpassDirectionalLight).GetDescriptor("GBufferDepth"), *depthBuffer);
	descSetInput->Write(SubpassPostProcessing, renderpass->GetSubpass(SubpassPostProcessing).GetDescriptor("HdrBuffer"), *textures[3]);
}

/*
This function is called when either of the following events occur:
- The initial framebuffer creation (after renderpass create).
- After the window resizes.
- When the window format changed.
*/
void Pu::DeferredRenderer::CreateFramebuffer(void)
{
	vector<const ImageView*> attachments;
	attachments.reserve(textures.size() + 1);

	for (const TextureInput2D *cur : textures) attachments.emplace_back(&cur->GetView());
	attachments.emplace_back(&depthBuffer->GetView());

	wnd->CreateFramebuffers(*renderpass, attachments);
}

void Pu::DeferredRenderer::DestroyWindowDependentResources(void)
{
	/* The textures store the images for us. */
	for (const TextureInput2D *cur : textures) delete cur;
	textures.clear();

	if (depthBuffer) delete depthBuffer;
}

void Pu::DeferredRenderer::DestroyPipelines(void)
{
	if (gfxTerrain) delete gfxTerrain;
	if (gfxGPassBasic) delete gfxGPassBasic;
	if (gfxGPassAdv) delete gfxGPassAdv;
	if (gfxGPassBasicMorph) delete gfxGPassBasicMorph;
	if (gfxDLight) delete gfxDLight;
	if (gfxPLight) delete gfxPLight;
	if (gfxSkybox) delete gfxSkybox;
	if (gfxTonePass) delete gfxTonePass;
	if (descSetInput) delete descSetInput;
	if (descPoolInput) delete descPoolInput;
}

void Pu::DeferredRenderer::Destroy(void)
{
	if (activeSubpass != SubpassNone) Log::Fatal("Cannot destroy deferred rendering during frame render!");

	DestroyWindowDependentResources();
	DestroyPipelines();

#ifdef _DEBUG
	/* Queries are always made. */
	delete timer;
	delete stats;
#endif

	delete lightVolumes;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}