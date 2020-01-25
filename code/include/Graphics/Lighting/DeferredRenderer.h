#pragma once
#include "Content/AssetFetcher.h"
#include "Graphics/Platform/GameWindow.h"
#include "Graphics/Models/Material.h"
#include "Graphics/Models/Mesh.h"
#include "DirectionalLight.h"
#include "Graphics/Cameras/Camera.h"

namespace Pu
{
	class Framebuffer;
	class CameraUniformBlock;

	/* Defines a deferred renderer that renders models in PBR to an HDR output. */
	class DeferredRenderer
	{
	public:
		/* Initializes a new instance of a deferred renderer for the specified window. */
		DeferredRenderer(_In_ AssetFetcher &fetcher, _In_ GameWindow &wnd, _In_ uint32 maxMaterials, _In_ uint32 maxLights);
		DeferredRenderer(_In_ const DeferredRenderer&) = delete;
		DeferredRenderer(_In_ DeferredRenderer&&) = delete;
		/* Releases the resources allocated by the deferred renderer. */
		~DeferredRenderer(void)
		{
			Destroy();
		}

		_Check_return_ DeferredRenderer& operator =(_In_ const DeferredRenderer&) = delete;
		_Check_return_ DeferredRenderer& operator =(_In_ DeferredRenderer&&) = delete;

		/* Checks whether the deferred renderer is ready for use. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return renderpass->IsLoaded();
		}

		/* Gets the pool from which PBR materials can be allocated. */
		_Check_return_ inline DescriptorPool& GetMaterialPool(void)
		{
			return *materialPool;
		}

		/* Gets the pool from which directional light can be allocated. */
		_Check_return_ inline DescriptorPool& GetLightPool(void)
		{
			return *lightPool;
		}

		/* Starts the deferred rendering pipeline. */
		void BeginGeometry(_In_ CommandBuffer &cmdBuffer, _In_ const Camera &camera);
		/* Starts the second phase of the deferred rendering pipeline. */
		void BeginLight(void);
		/* End the deferred rendering pipeline. */
		void End(void);
		/* Sets the model matrix for future meshes. */
		void SetModel(_In_ const Matrix &value);
		/* Renders the specified mesh with the specified material to the G-Buffer. */
		void Render(_In_ const Mesh &mesh, _In_ const Material &material);
		/* Renders the specified direction light onto the scene. */
		void Render(_In_ const DirectionalLight &light);

	private:
		Framebuffer *framebuffer;
		DepthBuffer *depthBuffer;
		Renderpass *renderpass;
		DescriptorPool *materialPool, *lightPool, *camPool;
		Image *gbuffAttach1, *gbuffAttach2, *gbuffAttach3, *gbuffAttach4, *tmpHdrAttach;
		vector<Texture2D*> textures;
		Sampler &sampler;
		
		AssetFetcher *fetcher;
		GameWindow *wnd;
		GraphicsPipeline *gfxGPass, *gfxLightPass, *gfxTonePass;
		CommandBuffer *curCmd;

		uint32 maxMaterials, maxLights;
		float hdrSwapchain;

		void DoTonemap(void);
		void OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs &args);
		void InitializeRenderpass(Renderpass&);
		void FinalizeRenderpass(Renderpass&);
		void CreateWindowDependentResources(void);
		void CreateFramebuffer(void);
		void DestroyWindowDependentResources(void);
		void Destroy(void);
	};
}