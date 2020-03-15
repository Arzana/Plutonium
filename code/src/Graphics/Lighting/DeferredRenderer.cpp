#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/VertexLayouts/Basic3D.h"
#include "Graphics/Diagnostics/QueryChain.h"
#include "Core/Diagnostics/Profiler.h"
#include "Graphics/Textures/TextureInput2D.h"

/*
	The shaders define the following descriptor sets:
	0: Camera
		Projection
		View
		Inverse Projection
		Inverse View
		Position
		Exposure
		Brightness
		Contrast
	1: Material
		Diffuse
		Specular / Glossiness
		Bump
		Emissive
		Occlusion
		F0 / Specular Power
		Diffuse Factor / Roughness
		Alpha Threshold
	2: Input Attachments
		Diffuse / Roughness^2
		Specular / Specular power
		Normal
		Emissive / AO
		HDR buffer
	3: Directional Light
		Direction
		Radiance (pre-multiplied)
		Environment map (temporary?)

	The framebuffer has several attachments:			G-Pass		Light-Pass		Sky-Pass	Post-Pass		Default Idx
	0: Swapchain				[r, g, b, a]			-			-				-			Color			5
	1: G-Buffer (Diffuse)		[r, g, b, a^2]			Color		Input			-			-				0
	2: G-Buffer (Specular)		[r, g, b, power]		Color		Input			-			-				1
	3: G-Buffer (Normal)		[x, y]					Color		Input			-			-				2
	4: HDR-Buffer				[r, g, b, a]			-			Color			Color		Input			0
	5: G-Buffer (Depth)			[d]						Depth		Input			Depth		-				3 & 1

	We need to override the attachment reference in most of the subpasses.
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, GameWindow & wnd)
	: wnd(&wnd), depthBuffer(nullptr), markNeeded(true), fetcher(&fetcher), skybox(nullptr),
	gfxGPass(nullptr), gfxLightPass(nullptr), gfxSkybox(nullptr), gfxTonePass(nullptr),
	curCmd(nullptr), curCam(nullptr), descPoolInput(nullptr), descSetInput(nullptr)
{
	/* We need to know if we'll be doing tone mapping or not. */
	hdrSwapchain = static_cast<float>(wnd.GetSwapchain().IsNativeHDR());

	/* Allocate all the profiler timers. */
	geometryTimer = new QueryChain(wnd.GetDevice(), QueryType::Timestamp);
	lightingTimer = new QueryChain(wnd.GetDevice(), QueryType::Timestamp);
	postTimer = new QueryChain(wnd.GetDevice(), QueryType::Timestamp);

	renderpass = &fetcher.FetchRenderpass(
		{
			{ L"{Shaders}StaticGeometry.vert.spv", L"{Shaders}Geometry.frag.spv" },
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

Pu::DescriptorPool * Pu::DeferredRenderer::CreateMaterialDescriptorPool(uint32 maxMaterials) const
{
	if (!renderpass->IsLoaded())
	{
		Log::Error("Unable to create descriptor pool from deferred renderer when renderpas is not yet loaded!");
		return nullptr;
	}

	/* Set 1 is the material set. */
	return new DescriptorPool(*renderpass, maxMaterials, 0, 1);
}

void Pu::DeferredRenderer::InitializeResources(CommandBuffer & cmdBuffer)
{
	/* Update the profiler and reset the queries. */
	Profiler::Begin("Rendering", Color::Red());
	Profiler::Add("Rendering (Geometry)", Color::Red(), geometryTimer->GetProfilerTimeDelta());
	Profiler::Add("Rendering (Lighting)", Color::Yellow(), lightingTimer->GetProfilerTimeDelta());
	Profiler::Add("Rendering (Post-Processing)", Color::Green(), postTimer->GetProfilerTimeDelta());

	geometryTimer->Reset(cmdBuffer);
	lightingTimer->Reset(cmdBuffer);
	postTimer->Reset(cmdBuffer);

	/* Make sure we only do this if needed. */
	curCmd = &cmdBuffer;
	if (!markNeeded) return;

	/* Mark all the framebuffer images as writable. */
	depthBuffer->MakeWritable(cmdBuffer);
	for (const TextureInput2D *attachment : textures)
	{
		cmdBuffer.MemoryBarrier(*attachment, PipelineStageFlag::TopOfPipe, PipelineStageFlag::ColorAttachmentOutput, ImageLayout::ColorAttachmentOptimal, AccessFlag::ColorAttachmentWrite, attachment->GetFullRange(), DependencyFlag::ByRegion);
	}
}

void Pu::DeferredRenderer::BeginGeometry(const Camera & camera)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCmd) Log::Fatal("InitializeResources should be called before the geometry pass can start!");
#endif

	/* Start the geometry pass. */
	curCam = &camera;
	curCmd->AddLabel("Deferred Renderer (Geometry)", Color::Blue());
	curCmd->BeginRenderPass(*renderpass, wnd->GetCurrentFramebuffer(*renderpass), SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxGPass);
	curCmd->BindGraphicsDescriptors(*gfxGPass, 0, camera);
	geometryTimer->RecordTimestamp(*curCmd, PipelineStageFlag::TopOfPipe);
}

void Pu::DeferredRenderer::BeginLight(void)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCam) Log::Fatal("Geometry pass should be started before the light pass can start!");
#endif

	/* End the geometry pass and start the directional light pass. */
	curCmd->EndLabel();
	geometryTimer->RecordTimestamp(*curCmd, PipelineStageFlag::BottomOfPipe);
	curCmd->AddLabel("Deferred Renderer (Directional Lights)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	lightingTimer->RecordTimestamp(*curCmd, PipelineStageFlag::TopOfPipe);
	curCmd->BindGraphicsPipeline(*gfxLightPass);
	curCmd->BindGraphicsDescriptors(*gfxLightPass, 1, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxLightPass, 1, *descSetInput);
}

void Pu::DeferredRenderer::End(void)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCam) Log::Fatal("Geometry pass should be started before the final pass can start!");
#endif

	/* End the light pass, do tonemapping and end the renderpass. */
	curCmd->EndLabel();
	lightingTimer->RecordTimestamp(*curCmd, PipelineStageFlag::BottomOfPipe);
	DoSkybox();
	DoTonemap();
	curCmd->EndRenderPass();

	/* Setting these to nullptr gives us the option to catch invalid usage. */
	curCmd = nullptr;
	curCmd = nullptr;
	Profiler::End();
}

void Pu::DeferredRenderer::SetModel(const Matrix & value)
{
	curCmd->PushConstants(*gfxGPass, ShaderStageFlag::Vertex, 0, sizeof(Matrix), value.GetComponents());
}

void Pu::DeferredRenderer::Render(const Mesh & mesh, const Material & material)
{
	curCmd->BindGraphicsDescriptor(*gfxGPass, material);
	mesh.Bind(*curCmd, 0);
	mesh.Draw(*curCmd);
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
		skybox = &renderpass->GetSubpass(2).GetDescriptor("Skybox");
		descSetInput->Write(2, *skybox, texture);
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
		curCmd->BindGraphicsPipeline(*gfxSkybox);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, 2, *curCam);
		curCmd->BindGraphicsDescriptors(*gfxSkybox, 2, *descSetInput);
		curCmd->Draw(3, 1, 0, 0);
		curCmd->EndLabel();
	}
	else curCmd->NextSubpass(SubpassContents::Inline);
}

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Camera Effects)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	postTimer->RecordTimestamp(*curCmd, PipelineStageFlag::TopOfPipe);
	curCmd->BindGraphicsPipeline(*gfxTonePass);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, 3, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, 3, *descSetInput);
	curCmd->PushConstants(*gfxTonePass, ShaderStageFlag::Fragment, 4, sizeof(float), &hdrSwapchain);
	curCmd->Draw(3, 1, 0, 0);
	curCmd->EndLabel();
	postTimer->RecordTimestamp(*curCmd, PipelineStageFlag::BottomOfPipe);
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
	/* Set all the options for the Geometry-Pass. */
	{
		Subpass &gpass = renderpass->GetSubpass(0);

		Output &depth = gpass.AddDepthStencil();
		depth.SetFormat(depthBuffer->GetFormat());
		depth.SetClearValue({ 1.0f, 0 });
		depth.SetLayouts(ImageLayout::DepthStencilAttachmentOptimal);
		depth.SetReference(5);

		Output &diffA2 = gpass.GetOutput("GBufferDiffuseA2");
		diffA2.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		diffA2.SetFormat(textures[0]->GetImage().GetFormat());
		diffA2.SetStoreOperation(AttachmentStoreOp::DontCare);
		diffA2.SetReference(1);

		Output &spec = gpass.GetOutput("GBufferSpecular");
		spec.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		spec.SetFormat(textures[1]->GetImage().GetFormat());
		spec.SetStoreOperation(AttachmentStoreOp::DontCare);
		spec.SetReference(2);

		Output &norm = gpass.GetOutput("GBufferNormal");
		norm.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		norm.SetFormat(textures[2]->GetImage().GetFormat());
		norm.SetStoreOperation(AttachmentStoreOp::DontCare);
		norm.SetReference(3);

		gpass.GetAttribute("Normal").SetOffset(vkoffsetof(Basic3D, Normal));
		gpass.GetAttribute("Tangent").SetOffset(vkoffsetof(Basic3D, Tangent));
		gpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Basic3D, TexCoord));
	}

	/* Set all the options for the directional light pass. */
	{
		Subpass &dlpass = renderpass->GetSubpass(1);
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
		Subpass &skypass = renderpass->GetSubpass(2);
		skypass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &depth = skypass.AddDepthStencil();
		depth.SetFormat(depthBuffer->GetFormat());
		depth.SetClearValue({ 1.0f, 0 });
		depth.SetLayouts(ImageLayout::DepthStencilAttachmentOptimal);
		depth.SetReference(5);

		Output &hdr = skypass.GetOutput("Hdr");
		hdr.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[3]->GetImage().GetFormat());
		hdr.SetLoadOperation(AttachmentLoadOp::DontCare);
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(4);
	}

	/* Set all the options for the camera pass. */
	{
		Subpass &fpass = renderpass->GetSubpass(3);
		fpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &screen = fpass.GetOutput("FragColor");
		screen.SetDescription(wnd->GetSwapchain());
		screen.SetLoadOperation(AttachmentLoadOp::DontCare);
		screen.SetReference(0);
	}
}

void Pu::DeferredRenderer::FinalizeRenderpass(Renderpass &)
{
	/* We need to delete the old pipelines if they are already created once. */
	if (gfxGPass || gfxLightPass || gfxSkybox || gfxTonePass)
	{
		delete gfxGPass;
		delete gfxLightPass;
		delete gfxSkybox;
		delete gfxTonePass;
	}
	else
	{
		/* We only need to create the descriptor set for the input attachments once. */
		descPoolInput = new DescriptorPool(*renderpass);
		descPoolInput->AddSet(1, 1, 1);
		descPoolInput->AddSet(2, 1, 1);
		descPoolInput->AddSet(3, 1, 1);

		descSetInput = new DescriptorSetGroup(*descPoolInput);
		descSetInput->Add(1, renderpass->GetSubpass(1).GetSetLayout(1));
		descSetInput->Add(2, renderpass->GetSubpass(2).GetSetLayout(1));
		descSetInput->Add(3, renderpass->GetSubpass(3).GetSetLayout(1));

		/* Write the input attachments to the descriptor set. */
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferDiffuseA2"), *textures[0]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferSpecular"), *textures[1]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferNormal"), *textures[2]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferDepth"), *depthBuffer);
		descSetInput->Write(3, renderpass->GetSubpass(3).GetDescriptor("HdrBuffer"), *textures[3]);
	}

	/* Create the graphics pipeline for the static geometry pass. */
	{
		gfxGPass = new GraphicsPipeline(*renderpass, 0);
		gfxGPass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPass->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPass->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPass->AddVertexBinding<Basic3D>(0);
		gfxGPass->Finalize();
	}

	/* Create the graphics pipeline for the directional light pass. */
	{
		gfxLightPass = new GraphicsPipeline(*renderpass, 1);
		gfxLightPass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxLightPass->SetTopology(PrimitiveTopology::TriangleList);
		gfxLightPass->Finalize();
	}

	/* Create the graphics pipeline for the skybox pass. */
	{
		gfxSkybox = new GraphicsPipeline(*renderpass, 2);
		gfxSkybox->SetViewport(wnd->GetNative().GetClientBounds());
		gfxSkybox->SetTopology(PrimitiveTopology::TriangleList);
		gfxSkybox->EnableDepthTest(false, CompareOp::LessOrEqual);
		gfxSkybox->Finalize();
	}

	/* Create the graphics pipeline for the camera pass. */
	{
		gfxTonePass = new GraphicsPipeline(*renderpass, 3);
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

void Pu::DeferredRenderer::Destroy(void)
{
	DestroyWindowDependentResources();

	/* Delete the graphics pipelines. */
	if (gfxGPass) delete gfxGPass;
	if (gfxLightPass) delete gfxLightPass;
	if (gfxSkybox) delete gfxSkybox;
	if (gfxTonePass) delete gfxTonePass;
	if (descSetInput) delete descSetInput;
	if (descPoolInput) delete descPoolInput;

	/* Queries are always made. */
	delete geometryTimer;
	delete lightingTimer;
	delete postTimer;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}