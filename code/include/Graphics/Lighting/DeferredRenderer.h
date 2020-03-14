#pragma once
#include "Graphics/Platform/GameWindow.h"
#include "Graphics/Models/Material.h"
#include "Graphics/Cameras/Camera.h"
#include "Graphics/Models/Mesh.h"
#include "Content/AssetFetcher.h"
#include "DirectionalLight.h"

namespace Pu
{
	class Framebuffer;
	class QueryChain;
	class TextureInput2D;

	/* Defines a deferred renderer that renders models in PBR to an HDR output. */
	class DeferredRenderer
	{
	public:
		/* Initializes a new instance of a deferred renderer for the specified window. */
		DeferredRenderer(_In_ AssetFetcher &fetcher, _In_ GameWindow &wnd);
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

		/* Gets the renderpass used by the deferred renderer. */
		_Check_return_ inline const Renderpass& GetRenderpass(void) const
		{
			return *renderpass;
		}

		/* Performs needed resource transitions. */
		void InitializeResources(_In_ CommandBuffer &cmdBuffer);
		/* Starts the deferred rendering pipeline. */
		void BeginGeometry(_In_ const Camera &camera);
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
		DepthBuffer *depthBuffer;
		Renderpass *renderpass;
		vector<TextureInput2D*> textures;
		
		AssetFetcher *fetcher;
		GameWindow *wnd;
		GraphicsPipeline *gfxGPass, *gfxLightPass, *gfxTonePass;

		DescriptorPool *descPoolInput;
		DescriptorSetGroup *descSetInput;

		CommandBuffer *curCmd;
		const Camera *curCam;
		QueryChain *geometryTimer, *lightingTimer, *postTimer;

		float hdrSwapchain;
		bool markNeeded;

		void DoTonemap(void);
		void OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs &args);
		void InitializeRenderpass(Renderpass&);
		void FinalizeRenderpass(Renderpass&);
		void CreateSizeDependentResources(void);
		void CreateFramebuffer(void);
		void DestroyWindowDependentResources(void);
		void Destroy(void);
	};
}