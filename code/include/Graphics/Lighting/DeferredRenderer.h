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
		/* Defines the index of the basic static geometry subpass. */
		static constexpr uint32 SubpassBasicStaticGeometry = 0;
		/* Defines the index of the advanced static geometry subpass. */
		static constexpr uint32 SubpassAdvancedStaticGeometry = 1;
		/* Defines the index of the directional light subpass. */
		static constexpr uint32 SubpassDirectionalLight = 2;
		/* Defines the index of the skybox subpass. */
		static constexpr uint32 SubpassSkybox = 3;
		/* Defines the index of the post-processing subpass. */
		static constexpr uint32 SubpassPostProcessing = 4;

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

		/* Gets the renderpass associated with this deferred renderer. */
		_Check_return_ inline const Renderpass& GetRenderpass(void) const
		{
			return *renderpass;
		}

		/* Gets the descriptor set layout for the materials used by the renderer. */
		_Check_return_ inline const DescriptorSetLayout& GetMaterialLayout(void) const
		{
			return renderpass->GetSubpass(SubpassAdvancedStaticGeometry).GetSetLayout(1);
		}

		/* Gets the descriptor set layout for the directional lights used by the renderer. */
		_Check_return_ inline const DescriptorSetLayout& GetDirectionalLightLayout(void) const
		{
			return renderpass->GetSubpass(SubpassDirectionalLight).GetSetLayout(2);
		}

		/* Creates a new descriptor poiol for materials rendered through this deferred renderer. */
		_Check_return_ DescriptorPool* CreateMaterialDescriptorPool(_In_ uint32 maxMaterials) const;
		/* Performs needed resource transitions. */
		void InitializeResources(_In_ CommandBuffer &cmdBuffer);
		/* Starts the deferred rendering pipeline (with basic static geometry). */
		void BeginGeometry(_In_ const Camera &camera);
		/* Starts the advanced section of the static geometry pipeline.s */
		void BeginAdvanced(void);
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
		/* Sets the skybox to use. */
		void SetSkybox(_In_ const TextureCube &texture);

	private:
		DepthBuffer *depthBuffer;
		Renderpass *renderpass;
		vector<TextureInput2D*> textures;
		
		AssetFetcher *fetcher;
		GameWindow *wnd;
		GraphicsPipeline *gfxGPassBasic, *gfxGPassAdv, *gfxLightPass, *gfxSkybox, *gfxTonePass;

		DescriptorPool *descPoolInput;
		DescriptorSetGroup *descSetInput;
		Descriptor *skybox;

		CommandBuffer *curCmd;
		const Camera *curCam;
		QueryChain *geometryTimer, *lightingTimer, *postTimer;

		float hdrSwapchain;
		bool markNeeded, advanced;

		void DoSkybox(void);
		void DoTonemap(void);
		void OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs &args);
		void InitializeRenderpass(Renderpass&);
		void FinalizeRenderpass(Renderpass&);
		void CreateSizeDependentResources(void);
		void WriteDescriptors(void);
		void CreateFramebuffer(void);
		void DestroyWindowDependentResources(void);
		void Destroy(void);
	};
}