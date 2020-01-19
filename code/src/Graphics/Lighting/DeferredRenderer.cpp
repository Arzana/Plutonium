#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/Cameras/CameraUniformBlock.h"

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
	: framebuffer(nullptr), materialPool(nullptr), lightPool(nullptr),
	fetcher(&fetcher), gfxGPass(nullptr), gfxLightPass(nullptr), gfxTonePass(nullptr), curCmd(nullptr),
	maxMaterials(maxMaterials), maxLights(maxLights), depthBuffer(nullptr)
{
	/* Some color spaces natively support HDR color spaces so we don't need a tone mapping pass. */
	if (hdrSwapchain = wnd.GetSwapchain().IsNativeHDR())
	{
		renderpass = &fetcher.FetchRenderpass(
			{
				{ L"{Shaders}StaticGeometry.vert.spv", L"{Shaders}Geometry.frag.spv" },
				{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}DirectionalLight.frag.spv" }
			});
	}
	else
	{
		renderpass = &fetcher.FetchRenderpass(
			{
				{ L"{Shaders}StaticGeometry.vert.spv", L"{Shaders}Geometry.frag.spv" },
				{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}DirectionalLight.frag.spv" },
				{ L"{Shaders}FullscreenQuad.vert.spv", L"{Shaders}Tonemap.frag.spv" }
			});
	}

	/* We need to recreate resources if the window resizes, or the color space is changed. */
	//wnd.SwapchainRecreated.Add(*this, &DeferredRenderer::OnSwapchainRecreated);
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
	UpdateCameraDescriptors(camera);
	cmdBuffer.BeginRenderPass(*renderpass, *framebuffer, SubpassContents::Inline);
	cmdBuffer.BindGraphicsPipeline(*gfxGPass);
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

	/* End the light pass, do tonemapping if needed and end the renderpass. */
	curCmd->EndLabel();
	if (!hdrSwapchain) DoTonemap();
	curCmd->EndRenderPass();
}

void Pu::DeferredRenderer::SetModel(const Matrix & value)
{
	curCmd->PushConstants(*renderpass, ShaderStageFlag::Vertex, sizeof(Matrix), value.GetComponents());
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

void Pu::DeferredRenderer::UpdateCameraDescriptors(const Camera & camera)
{
	camBlock->SetProjection(camera.GetProjection());
	camBlock->SetView(camera.GetView());
	camBlock->Update(*curCmd);
}

void Pu::DeferredRenderer::DoTonemap(void)
{
	curCmd->AddLabel("Deferred Renderer (Tone Mapping)", Color::Blue());
	curCmd->NextSubpass(SubpassContents::Inline);
	curCmd->BindGraphicsPipeline(*gfxTonePass);
	curCmd->Draw(3, 1, 0, 0);
	curCmd->EndLabel();
}

void Pu::DeferredRenderer::OnSwapchainRecreated(const GameWindow & wnd)
{
	//TODO: find a way to handle this gracefully.
	if (renderpass->IsLoaded())
	{
		Log::Fatal("Deferred render is invalid!");
	}
}

void Pu::DeferredRenderer::InitializeRenderpass(Renderpass &)
{

}