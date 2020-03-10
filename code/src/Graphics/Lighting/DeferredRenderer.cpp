#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/VertexLayouts/SkinnedAnimated.h"

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

	The framebuffer has several attachments:			G-Pass		Light-Pass		Post-Pass		Default Idx
	0: Swapchain				[r, g, b, a]			-			-				Color			5
	1: G-Buffer (Diffuse)		[r, g, b, a^2]			Color		Input			-				0
	2: G-Buffer (Specular)		[r, g, b, power]		Color		Input			-				1
	3: G-Buffer (Normal)		[x, y]					Color		Input			-				2
	4: G-Buffer (Emissive)		[r, g, b, ao]			Color		Input			-				3
	5: HDR-Buffer				[r, g, b, a]			-			Color			Input			0
	6: G-Buffer (Depth)			[d]						Depth		Input			-				4

	We need to override the attachment reference in most of the subpasses.
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, GameWindow & wnd)
	: wnd(&wnd), depthBuffer(nullptr), markNeeded(true), fetcher(&fetcher),
	gfxGPass(nullptr), gfxLightPass(nullptr), gfxTonePass(nullptr), 
	curCmd(nullptr), curCam(nullptr), descPoolInput(nullptr), descSetInput(nullptr)
{
	/* We need to know if we'll be doing tone mapping or not. */
	hdrSwapchain = static_cast<float>(wnd.GetSwapchain().IsNativeHDR());

	renderpass = &fetcher.FetchRenderpass(
		{
			{ L"{Shaders}StaticGeometry.vert.spv", L"{Shaders}Geometry.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}DirectionalLight.frag.spv" },
			{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}CameraEffects.frag.spv" }
		});

	/* We need to recreate resources if the window resizes, or the color space is changed. */
	wnd.SwapchainRecreated.Add(*this, &DeferredRenderer::OnSwapchainRecreated);
	renderpass->PreCreate.Add(*this, &DeferredRenderer::InitializeRenderpass);
	renderpass->PostCreate.Add(*this, &DeferredRenderer::FinalizeRenderpass);
	CreateSizeDependentResources();
}

void Pu::DeferredRenderer::InitializeResources(CommandBuffer & cmdBuffer)
{
	/* Make sure we only do this if needed. */
	curCmd = &cmdBuffer;
	if (!markNeeded) return;
	curCmd->AddLabel("Deferred Renderer (Initialization)", Color::Blue());

	/* Mark all the framebuffer images as writable. */
	depthBuffer->MakeWritable(cmdBuffer);
	for (const TextureInput2D *attachment : textures)
	{
		cmdBuffer.MemoryBarrier(*attachment, PipelineStageFlag::TopOfPipe, PipelineStageFlag::ColorAttachmentOutput, ImageLayout::ColorAttachmentOptimal, AccessFlag::ColorAttachmentWrite, attachment->GetFullRange(), DependencyFlag::ByRegion);
	}

	curCmd->EndLabel();
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
}

void Pu::DeferredRenderer::BeginLight(void)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCam) Log::Fatal("Geometry pass should be started before the light pass can start!");
#endif

	/* End the geometry pass and start the directional light pass. */
	curCmd->EndLabel();
	curCmd->AddLabel("Deferred Renderer (Directional Lights)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
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
	DoTonemap();
	curCmd->EndRenderPass();
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

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Camera Effects)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxTonePass);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, 2, *curCam);
	curCmd->BindGraphicsDescriptors(*gfxTonePass, 2, *descSetInput);
	curCmd->PushConstants(*gfxTonePass, ShaderStageFlag::Fragment, 4, sizeof(float), &hdrSwapchain);
	curCmd->Draw(3, 1, 0, 0);
	curCmd->EndLabel();
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
		depth.SetReference(6);

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

		Output &emissAo = gpass.GetOutput("GBufferEmissiveAO");
		emissAo.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		emissAo.SetFormat(textures[3]->GetImage().GetFormat());
		emissAo.SetStoreOperation(AttachmentStoreOp::DontCare);
		emissAo.SetReference(4);

		gpass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
		gpass.GetAttribute("Tangent").SetOffset(vkoffsetof(SkinnedAnimated, Tangent));
		gpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));
	}

	/* Set all the options for the directional light pass. */
	{
		Subpass &dlpass = renderpass->GetSubpass(1);
		dlpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &hdr = dlpass.GetOutput("L0");
		hdr.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[4]->GetImage().GetFormat());
		hdr.SetLoadOperation(AttachmentLoadOp::DontCare);
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(5);
	}

	/* Set all the options for the camera pass. */
	{
		Subpass &fpass = renderpass->GetSubpass(2);
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
	if (gfxGPass || gfxLightPass || gfxTonePass)
	{
		delete gfxGPass;
		delete gfxLightPass;
		delete gfxTonePass;
	}
	else
	{
		/* We only need to create the descriptor set for the input attachments once. */
		descPoolInput = new DescriptorPool(*renderpass);
		descPoolInput->AddSet(1, 1, 1);
		descPoolInput->AddSet(2, 1, 1);

		descSetInput = new DescriptorSetGroup(*descPoolInput);
		descSetInput->Add(1, renderpass->GetSubpass(1).GetSetLayout(1));
		descSetInput->Add(2, renderpass->GetSubpass(2).GetSetLayout(1));

		/* Write the input attachments to the descriptor set. */
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferDiffuseA2"), *textures[0]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferSpecular"), *textures[1]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferNormal"), *textures[2]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferEmissiveAO"), *textures[3]);
		descSetInput->Write(1, renderpass->GetSubpass(1).GetDescriptor("GBufferDepth"), *depthBuffer);
		descSetInput->Write(2, renderpass->GetSubpass(2).GetDescriptor("HdrBuffer"), *textures[4]);
	}

	/* Create the graphics pipeline for the static geometry pass. */
	{
		gfxGPass = new GraphicsPipeline(*renderpass, 0);
		gfxGPass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxGPass->SetTopology(PrimitiveTopology::TriangleList);
		gfxGPass->EnableDepthTest(true, CompareOp::LessOrEqual);
		gfxGPass->AddVertexBinding<SkinnedAnimated>(0);
		gfxGPass->Finalize();
	}

	/* Create the graphics pipeline for the directional light pass. */
	{
		gfxLightPass = new GraphicsPipeline(*renderpass, 1);
		gfxLightPass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxLightPass->SetTopology(PrimitiveTopology::TriangleList);
		gfxLightPass->SetCullMode(CullModeFlag::Front);
		gfxLightPass->Finalize();
	}

	/* Create the graphics pipeline for the directional camera pass. */
	{
		gfxTonePass = new GraphicsPipeline(*renderpass, 2);
		gfxTonePass->SetViewport(wnd->GetNative().GetClientBounds());
		gfxTonePass->SetTopology(PrimitiveTopology::TriangleList);
		gfxTonePass->SetCullMode(CullModeFlag::Front);
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
	Image *gbuffAttach4 = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R8G8B8A8_UNORM, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));
	Image *tmpHdrAttach = new Image(device, ImageCreateInfo(ImageType::Image2D, Format::R16G16B16A16_SFLOAT, size, 1, 1, SampleCountFlag::Pixel1Bit, gbufferUsage));

	/* Create the new image views and samplers. */
	textures.emplace_back(new TextureInput2D(*gbuffAttach1));
	textures.emplace_back(new TextureInput2D(*gbuffAttach2));
	textures.emplace_back(new TextureInput2D(*gbuffAttach3));
	textures.emplace_back(new TextureInput2D(*gbuffAttach4));
	textures.emplace_back(new TextureInput2D(*tmpHdrAttach));
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
	if (gfxTonePass) delete gfxTonePass;
	if (descSetInput) delete descSetInput;
	if (descPoolInput) delete descPoolInput;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}