#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/VertexLayouts/Advanced3D.h"
#include "Graphics/Textures/TextureInput2D.h"
#include "Graphics/VertexLayouts/Patched3D.h"
#include "Graphics/Diagnostics/QueryChain.h"
#include "Graphics/VertexLayouts/Basic3D.h"
#include "Core/Diagnostics/Profiler.h"

/* Check for invalid state on debug. */
#ifdef _DEBUG
#define DBG_CHECK(obj, stage)	if (!obj) Log::Fatal("InitializeResources should be started before the " stage " pass can start!");
#else
#define DBG_CHECK_CAM(...)
#endif

constexpr Pu::uint32 TerrainTimer = 0;
constexpr Pu::uint32 GeometryTimer = 1;
constexpr Pu::uint32 SkyboxTimer = 2;
constexpr Pu::uint32 LightingTimer = 3;
constexpr Pu::uint32 PostTimer = 4;

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
	3: G-Buffer (Normal)		[x, y]					Color		Input			-			-				2
	4: HDR-Buffer				[r, g, b, a]			-			Color			Color		Input			0
	5: G-Buffer (Depth)			[d]						Depth		Input			Depth		-				3 & 1

	We need to override the attachment reference in most of the subpasses.
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, GameWindow & wnd, bool wireframe)
	: wnd(&wnd), depthBuffer(nullptr), markNeeded(true), fetcher(&fetcher), skybox(nullptr),
	gfxTerrain(nullptr), gfxGPassBasic(nullptr), gfxGPassAdv(nullptr), gfxLightPass(nullptr),
	gfxSkybox(nullptr), gfxTonePass(nullptr), curCmd(nullptr), curCam(nullptr), wireframe(wireframe),
	descPoolInput(nullptr), descSetInput(nullptr), advanced(false)
{
	timer = new QueryChain(wnd.GetDevice(), QueryType::Timestamp, 5);

	/* Make sure that we can actually run wireframe mode. */
	if (wireframe)
	{
		if (!wnd.GetDevice().GetPhysicalDevice().GetEnabledFeatures().FillModeNonSolid)
		{
			Log::Warning("Cannot use DeferredRenderer in wireframe mode (FillModeNonSolid was not enabled)!");
			wireframe = false;
		}
	}

	renderpass = &fetcher.FetchRenderpass(
		{
			{ L"{Shaders}Terrain.vert.spv", L"{Shaders}Terrain.tesc.spv", L"{Shaders}Terrain.tese.spv", L"{Shaders}Terrain.frag.spv" },
			{ L"{Shaders}BasicStaticGeometry.vert.spv", L"{Shaders}BasicGeometry.frag.spv" },
			{ L"{Shaders}AdvancedStaticGeometry.vert.spv", L"{Shaders}AdvancedGeometry.frag.spv" },
			{ L"{Shaders}BasicMorphGeometry.vert.spv", L"{Shaders}BasicGeometry.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}DirectionalLight.frag.spv" },
			{ L"{Shaders}Skybox.vert.spv", L"{Shaders}Skybox.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}CameraEffects.frag.spv" }
		});

	/* We need to recreate resources if the window resizes, or the color space is changed. */
	wnd.SwapchainRecreated.Add(*this, &DeferredRenderer::OnSwapchainRecreated);
	renderpass->PreCreate.Add(*this, &DeferredRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DeferredRenderer::FinalizeRenderpass);
	CreateSizeDependentResources();
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
	pool.AddSet(SubpassTerrain, 0, maxSets);				// Terrain
	pool.AddSet(SubpassAdvancedStaticGeometry, 0, maxSets);	// Static Geometry
	pool.AddSet(SubpassDirectionalLight, 0, maxSets);		// Directional Light
	pool.AddSet(SubpassSkybox, 0, maxSets);					// Skybox
	pool.AddSet(SubpassPostProcessing, 0, maxSets);			// Post-Processing
}

void Pu::DeferredRenderer::InitializeResources(CommandBuffer & cmdBuffer, const Camera & camera)
{
	/* Update the profiler and reset the queries. */
	Profiler::Begin("Rendering", Color::Red());
	Profiler::Add("Rendering (Environment)", Color::Gray(), timer->GetProfilerTimeDelta(TerrainTimer) + timer->GetProfilerTimeDelta(SkyboxTimer));
	Profiler::Add("Rendering (Geometry)", Color::Red(), timer->GetProfilerTimeDelta(GeometryTimer));
	Profiler::Add("Rendering (Lighting)", Color::Yellow(), timer->GetProfilerTimeDelta(LightingTimer));
	Profiler::Add("Rendering (Post-Processing)", Color::Green(), timer->GetProfilerTimeDelta(PostTimer));

	timer->Reset(cmdBuffer);

	/* Make sure we only do this if needed. */
	curCmd = &cmdBuffer;
	curCam = &camera;
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

void Pu::DeferredRenderer::BeginTerrain(void)
{
	DBG_CHECK(curCmd, "Terrain");

	curCmd->AddLabel("Deferred Renderer (Terrain)", Color::Blue());
	curCmd->BeginRenderPass(*renderpass, wnd->GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxTerrain);
	curCmd->BindGraphicsDescriptors(*gfxTerrain, SubpassTerrain, *curCam);
	timer->RecordTimestamp(*curCmd, TerrainTimer, PipelineStageFlag::TopOfPipe);
}

void Pu::DeferredRenderer::BeginGeometry(void)
{
	DBG_CHECK(curCam, "Basic-Geomtry");

	/* End the terrain pass and start the geometry pass. */
	advanced = false;
	curCmd->EndLabel();
	timer->RecordTimestamp(*curCmd, TerrainTimer, PipelineStageFlag::BottomOfPipe);
	curCmd->AddLabel("Deferred Renderer (Basic Static Geometry)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxGPassBasic);
	curCmd->BindGraphicsDescriptors(*gfxGPassBasic, SubpassAdvancedStaticGeometry, *curCam);
	timer->RecordTimestamp(*curCmd, GeometryTimer, PipelineStageFlag::TopOfPipe);
}

void Pu::DeferredRenderer::BeginAdvanced(void)
{
	DBG_CHECK(curCam, "Advanced-Geomtry");

	/* End the basic pass and start the advanced pass. */
	advanced = true;
	curCmd->EndLabel();
	curCmd->AddLabel("Deferred Renderer (Advanced Static Geometry)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxGPassAdv);
	curCmd->BindGraphicsDescriptors(*gfxGPassAdv, SubpassAdvancedStaticGeometry, *curCam);
}

void Pu::DeferredRenderer::BeginMorph(void)
{
	DBG_CHECK(curCam, "Morph-Geometry");

	/* End the advanced pass and start the morph pass. */
	curCmd->EndLabel();
	curCmd->AddLabel("Deferred Renderer (Basic Morph Geometry)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxGPassBasicMorph);
	curCmd->BindGraphicsDescriptors(*gfxGPassBasicMorph, SubpassAdvancedStaticGeometry, *curCam);
}

void Pu::DeferredRenderer::BeginLight(void)
{
	DBG_CHECK(curCam, "Directional-Light");

	/* End the geometry pass and start the directional light pass. */
	curCmd->EndLabel();
	timer->RecordTimestamp(*curCmd, GeometryTimer, PipelineStageFlag::BottomOfPipe);
	curCmd->AddLabel("Deferred Renderer (Directional Lights)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	timer->RecordTimestamp(*curCmd, LightingTimer, PipelineStageFlag::TopOfPipe);
	curCmd->BindGraphicsPipeline(*gfxLightPass);
	curCmd->BindGraphicsDescriptors(*gfxLightPass, SubpassDirectionalLight, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxLightPass, SubpassDirectionalLight, *descSetInput);
}

void Pu::DeferredRenderer::End(void)
{
	DBG_CHECK(curCam, "final");

	/* End the light pass, do tonemapping and end the renderpass. */
	curCmd->EndLabel();
	timer->RecordTimestamp(*curCmd, LightingTimer, PipelineStageFlag::BottomOfPipe);
	DoSkybox();
	DoTonemap();
	curCmd->EndRenderPass();

	/* Setting these to nullptr gives us the option to catch invalid usage. */
	curCmd = nullptr;
	curCmd = nullptr;
	Profiler::End();
}

void Pu::DeferredRenderer::Render(const TerrainChunk & chunk)
{
	if (curCam->Cull(chunk.GetBoundingBox())) return;

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
	curCmd->BindGraphicsDescriptor(*gfxLightPass, light);
	curCmd->Draw(3, 1, 0, 0);
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

void Pu::DeferredRenderer::DoSkybox(void)
{
	/* Skip rendering the skybox if none was set. */
	if (skybox)
	{
		curCmd->AddLabel("Deferred Renderer (Skybox)", Color::Blue());
		curCmd->NextSubpass(SubpassContents::Inline);
		timer->RecordTimestamp(*curCmd, SkyboxTimer, PipelineStageFlag::TopOfPipe);
		curCmd->BindGraphicsPipeline(*gfxSkybox);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, SubpassSkybox, *curCam);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, SubpassSkybox, *descSetInput);
		curCmd->Draw(3, 1, 0, 0);
		curCmd->EndLabel();
		timer->RecordTimestamp(*curCmd, SkyboxTimer, PipelineStageFlag::BottomOfPipe);
	}
	else curCmd->NextSubpass(SubpassContents::Inline);
}

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Camera Effects)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	timer->RecordTimestamp(*curCmd, PostTimer, PipelineStageFlag::TopOfPipe);
	curCmd->BindGraphicsPipeline(*gfxTonePass);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, SubpassPostProcessing, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, SubpassPostProcessing, *descSetInput);
	curCmd->Draw(3, 1, 0, 0);
	curCmd->EndLabel();
	timer->RecordTimestamp(*curCmd, PostTimer, PipelineStageFlag::BottomOfPipe);
}

void Pu::DeferredRenderer::OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs & args)
{
	/* We can just ignore the event if the renderpass isn't loaded yet. */
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
		dlpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &hdr = dlpass.GetOutput("L0");
		hdr.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[3]->GetImage().GetFormat());
		hdr.SetLoadOperation(AttachmentLoadOp::DontCare);
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(4);
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
		gfxLightPass = new GraphicsPipeline(*renderpass, SubpassDirectionalLight);
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

		gfxTerrain->SetViewport(wnd->GetNative().GetClientBounds());
		gfxTerrain->SetTopology(PrimitiveTopology::PatchList);
		gfxTerrain->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxTerrain->SetCullMode(CullModeFlag::Back);
		gfxTerrain->AddVertexBinding<Patched3D>(0);
		gfxTerrain->SetPatchControlPoints(4);
		gfxTerrain->Finalize();
	}

	/* Create the graphics pipeline for the basic static geometry pass. */
	{
		if (wireframe) gfxGPassBasic->SetPolygonMode(PolygonMode::Line);

		gfxGPassBasic->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPassBasic->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPassBasic->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPassBasic->AddVertexBinding<Basic3D>(0);
		gfxGPassBasic->Finalize();
	}

	/* Create the graphics pipeline for the advanced static geometry pass. */
	{
		if (wireframe) gfxGPassAdv->SetPolygonMode(PolygonMode::Line);

		gfxGPassAdv->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPassAdv->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPassAdv->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPassAdv->AddVertexBinding<Advanced3D>(0);
		gfxGPassAdv->Finalize();
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
	}

	/* Create the graphics pipeline for the directional light pass. */
	{
		gfxLightPass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxLightPass->SetTopology(PrimitiveTopology::TriangleList);
		gfxLightPass->GetBlendState("L0").SetBlendFactors(BlendFactor::One, BlendFactor::One, BlendFactor::One, BlendFactor::One);
		gfxLightPass->Finalize();
	}

	/* Create the graphics pipeline for the skybox pass. */
	{
		gfxSkybox->SetViewport(wnd->GetNative().GetClientBounds());
		gfxSkybox->SetTopology(PrimitiveTopology::TriangleList);
		gfxSkybox->EnableDepthTest(false, CompareOp::LessOrEqual);
		gfxSkybox->Finalize();
	}

	/* Create the graphics pipeline for the camera pass. */
	{
		gfxTonePass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxTonePass->SetTopology(PrimitiveTopology::TriangleList);
		gfxTonePass->Finalize();
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
	Image *gbuffAttach3 = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R16G16_SFLOAT, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));
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
	if (gfxLightPass) delete gfxLightPass;
	if (gfxSkybox) delete gfxSkybox;
	if (gfxTonePass) delete gfxTonePass;
	if (descSetInput) delete descSetInput;
	if (descPoolInput) delete descPoolInput;
}

void Pu::DeferredRenderer::Destroy(void)
{
	DestroyWindowDependentResources();
	DestroyPipelines();

	/* Queries are always made. */
	delete timer;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}