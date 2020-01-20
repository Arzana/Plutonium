#include "Graphics/Lighting/DeferredRenderer.h"

/*
	The shaders define the following descriptor sets:
	0: Camera
		Projection
		View
		Inverse Projection
		Inverse View
		Position
		Exposure	(Only available in LDR mode)
		Brightness	(Only available in LDR mode)
		Contrast	(Only available in LDR mode)
	1: Material
		Diffuse
		Specular / Glossiness
		Bump
		Emissive
		Occlusion
		F0 / Specular Power
		Diffuse Factor / Roughness
		Alpha Threshold
	2: G-Buffer
		Diffuse / Roughness^2
		Specular / Specular power
		Normal
		Emissive / AO
	3: Directional Light
		Direction
		Radiance (pre-multiplied)
	4: HDR Buffer (Only available in LDR mode)
*/
Pu::DeferredRenderer::DeferredRenderer(AssetFetcher & fetcher, const GameWindow & wnd, uint32 maxMaterials, uint32 maxLights)
	: framebuffer(nullptr), materialPool(nullptr), lightPool(nullptr), wnd(&wnd),
	fetcher(&fetcher), gfxGPass(nullptr), gfxLightPass(nullptr), gfxTonePass(nullptr), curCmd(nullptr),
	maxMaterials(maxMaterials), maxLights(maxLights), depthBuffer(nullptr)
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
}

void Pu::DeferredRenderer::BeginGeometry(CommandBuffer & cmdBuffer, const Camera & camera)
{
	/* Sets the command buffer. */
	if (curCmd) Log::Fatal("Geometry pass has already been started!");
	curCmd = &cmdBuffer;

	/* Start the geometry pass. */
	cmdBuffer.AddLabel("Deferred Renderer (Geometry)", Color::Blue());

	cmdBuffer.BeginRenderPass(*renderpass, *framebuffer, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*gfxGPass);
	cmdBuffer.BindGraphicsDescriptor(camera);
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
	curCmd->BindGraphicsPipeline(*gfxLightPass);
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
	curCmd->BindGraphicsPipeline(*gfxTonePass);
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
		but we only have to recreate the framebuffers if the area changed, because we use a dynamic viewport.
		*/
		if (args.FormatChanged) renderpass->Recreate();
		if (args.AreaChanged) CreateWindowDependentResources();
	}
}

void Pu::DeferredRenderer::Destroy(void)
{
	/* Delete the G-Buffer, HDR buffer and the framebuffer. */
	if (framebuffer) delete framebuffer;
	if (gbuffAttach1) delete gbuffAttach1;
	if (gbuffAttach2) delete gbuffAttach2;
	if (gbuffAttach3) delete gbuffAttach3;
	if (gbuffAttach4) delete gbuffAttach4;
	if (tmpHdrAttach) delete tmpHdrAttach;
	if (depthBuffer) delete depthBuffer;

	/* Delete the descriptor pools. */
	if (camPool) delete camPool;
	if (lightPool) delete lightPool;
	if (materialPool) delete materialPool;

	/* Delete the graphics pipelines. */
	if (gfxGPass) delete gfxGPass;
	if (gfxLightPass) delete gfxLightPass;
	if (gfxTonePass) delete gfxTonePass;

	/* Release the renderpass to the content manager and remove the event handler for window resizes. */
	fetcher->Release(*renderpass);
	wnd->SwapchainRecreated.Remove(*this, &DeferredRenderer::OnSwapchainRecreated);
}