#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/VertexLayouts/Basic3D.h"

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
	0: G-Buffer (Diffuse)		[r, g, b, a^2]			Color		Input			-				0
	1: G-Buffer (Specular)		[r, g, b, power]		Color		Input			-				1
	2: G-Buffer (Normal)		[x, y]					Color		Input			-				2
	3: G-Buffer (Emissive)		[r, g, b, ao]			Color		Input			-				3
	5: G-Buffer (Depth)			[d]						Color		Input			-				4
	4: HDR-Buffer				[r, g, b, a]			-			Color			Input			0
	6: Swapchain				[r, g, b, a]			-			-				Color			0

	We need to override the attachment reference in most of the subpasses.
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, GameWindow & wnd)
	: framebuffer(nullptr), wnd(&wnd), depthBuffer(nullptr), markNeeded(true),
	fetcher(&fetcher), gfxGPass(nullptr), gfxFullScreen(nullptr), curCmd(nullptr)
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
	curCmd->AddLabel("Deferred Renderer (Geometry)", Color::Blue());
	curCmd->BeginRenderPass(*renderpass, *framebuffer, SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxGPass);
	curCmd->BindGraphicsDescriptor(camera);
}

void Pu::DeferredRenderer::BeginLight(void)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCmd) Log::Fatal("Geometry pass should be started before the light pass can start!");
#endif

	/* End the geometry pass and start the directional light pass. */
	curCmd->EndLabel();
	curCmd->AddLabel("Deferred Renderer (Directional Lights)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxFullScreen);
}

void Pu::DeferredRenderer::End(void)
{
	/* Check for invalid state on debug. */
#ifdef _DEBUG
	if (!curCmd) Log::Fatal("Geometry pass should be started before the final pass can start!");
#endif

	/* End the light pass, do tonemapping and end the renderpass. */
	curCmd->EndLabel();
	DoTonemap();
	curCmd->EndRenderPass();
}

void Pu::DeferredRenderer::SetModel(const Matrix & value)
{
	curCmd->PushConstants(*renderpass, ShaderStageFlag::Vertex, 0, sizeof(Matrix), value.GetComponents());
}

void Pu::DeferredRenderer::Render(const Mesh & mesh, const Material & material)
{
	curCmd->BindGraphicsDescriptor(material);
	mesh.Bind(*curCmd, 0);
	mesh.Draw(*curCmd);
}

void Pu::DeferredRenderer::Render(const DirectionalLight & light)
{
	curCmd->BindGraphicsDescriptor(light);
	curCmd->Draw(3, 1, 0, 0);
}

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Camera Effects)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxFullScreen);
	curCmd->PushConstants(*renderpass, ShaderStageFlag::Fragment, 4, sizeof(float), &hdrSwapchain);
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
		depth.SetLayouts(ImageLayout::DepthStencilAttachmentOptimal, ImageLayout::DepthStencilAttachmentOptimal, ImageLayout::DepthStencilReadOnlyOptimal);

		Output &diffA2 = gpass.GetOutput("GBufferDiffuseA2");
		diffA2.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		diffA2.SetFormat(textures[0]->GetImage().GetFormat());
		diffA2.SetStoreOperation(AttachmentStoreOp::DontCare);

		Output &spec = gpass.GetOutput("GBufferSpecular");
		spec.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		spec.SetFormat(textures[1]->GetImage().GetFormat());
		spec.SetStoreOperation(AttachmentStoreOp::DontCare);

		Output &norm = gpass.GetOutput("GBufferNormal");
		norm.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		norm.SetFormat(textures[2]->GetImage().GetFormat());
		norm.SetStoreOperation(AttachmentStoreOp::DontCare);

		Output &emissAo = gpass.GetOutput("GBufferEmissiveAO");
		emissAo.SetLayouts(ImageLayout::ColorAttachmentOptimal);
		emissAo.SetFormat(textures[3]->GetImage().GetFormat());
		emissAo.SetStoreOperation(AttachmentStoreOp::DontCare);

		gpass.GetAttribute("Normal").SetOffset(vkoffsetof(Basic3D, Normal));
		gpass.GetAttribute("Tangent").SetOffset(vkoffsetof(Basic3D, Tangent));
		gpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(Basic3D, TexCoord));
	}

	/* Set all the options for the directional light pass. */
	{
		Subpass &dlpass = renderpass->GetSubpass(1);
		dlpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &hdr = dlpass.GetOutput("L0");
		hdr.SetLayout(ImageLayout::ColorAttachmentOptimal);
		hdr.SetFormat(textures[4]->GetImage().GetFormat());
		hdr.SetLoadOperation(AttachmentLoadOp::DontCare);
		hdr.SetStoreOperation(AttachmentStoreOp::DontCare);
		hdr.SetReference(4);
	}

	/* Set all the options for the camera pass. */
	{
		Subpass &fpass = renderpass->GetSubpass(2);
		fpass.SetDependency(PipelineStageFlag::ColorAttachmentOutput, PipelineStageFlag::FragmentShader, AccessFlag::ColorAttachmentWrite, AccessFlag::InputAttachmentRead, DependencyFlag::ByRegion);

		Output &screen = fpass.GetOutput("FragColor");
		screen.SetDescription(wnd->GetSwapchain());
		screen.SetLoadOperation(AttachmentLoadOp::DontCare);
		screen.SetReference(6);
	}
}

void Pu::DeferredRenderer::FinalizeRenderpass(Renderpass &)
{
	/* We need to delete the old pipelines if they are already created once. */
	if(gfxGPass || gfxFullScreen)
	{
		delete gfxGPass;
		delete gfxFullScreen;
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

	/* Create the graphics pipeline for the directional light and camera pass. */
	{
		gfxFullScreen = new GraphicsPipeline(*renderpass, 1);
		gfxFullScreen->SetViewport(wnd->GetNative().GetClientBounds());
		gfxFullScreen->SetTopology(PrimitiveTopology::TriangleList);
		gfxFullScreen->SetCullMode(CullModeFlag::Front);
		gfxFullScreen->Finalize();
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
	vector<const ImageView*> attachments{ textures.size() + 1 };
	for (const TextureInput2D *cur : textures) attachments.emplace_back(&cur->GetView());
	attachments.emplace_back(&depthBuffer->GetView());

	wnd->CreateFramebuffers(*renderpass, attachments);
}

void Pu::DeferredRenderer::DestroyWindowDependentResources(void)
{
	/* Delete the G-Buffer, HDR buffer and the framebuffer. */
	if (framebuffer) delete framebuffer;

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
	if (gfxFullScreen) delete gfxFullScreen;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}