#pragma once
#include "Graphics/Platform/GameWindow.h"
#include "Graphics/Cameras/Camera.h"
#include "Graphics/Models/Terrain.h"
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
		/* Defines the index of the terrain subpass. */
		static constexpr uint32 SubpassTerrain = 0;
		/* Defines the index of the basic static geometry subpass. */
		static constexpr uint32 SubpassBasicStaticGeometry = 1;
		/* Defines the index of the advanced static geometry subpass. */
		static constexpr uint32 SubpassAdvancedStaticGeometry = 2;
		/* Defines the index of the directional light subpass. */
		static constexpr uint32 SubpassDirectionalLight = 3;
		/* Defines the index of the skybox subpass. */
		static constexpr uint32 SubpassSkybox = 4;
		/* Defines the index of the post-processing subpass. */
		static constexpr uint32 SubpassPostProcessing = 5;

		/* Initializes a new instance of a deferred renderer for the specified window. */
		DeferredRenderer(_In_ AssetFetcher &fetcher, _In_ GameWindow &wnd, _In_opt_ bool wireframe = false);
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

		/* Gets the descriptor set layout for the terrain details. */
		_Check_return_ inline const DescriptorSetLayout& GetTerrainLayout(void) const
		{
			return renderpass->GetSubpass(SubpassTerrain).GetSetLayout(1);
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

		/* Gets the depth buffer associated with the deferred renderer. */
		_Check_return_ inline const DepthBuffer& GetDepthBuffer(void) const
		{
			return *depthBuffer;
		}
 
		/* Creates a new descriptor pool for terrains. */
		_Check_return_ DescriptorPool* CreateTerrainDescriptorPool(_In_ uint32 maxTerrains) const;
		/* Creates a new descriptor pool for materials rendered through this deferred renderer. */
		_Check_return_ DescriptorPool* CreateMaterialDescriptorPool(_In_ uint32 maxMaterials) const;
		/* Initializes a descriptor pool for use with the deferred renderer cameras. */
		void InitializeCameraPool(_In_ DescriptorPool &pool, _In_ uint32 maxSets) const;
		/* Performs needed resource transitions. */
		void InitializeResources(_In_ CommandBuffer &cmdBuffer);
		/* Started the deferred rendering pipeline (with terrain rendering). */
		void BeginTerrain(_In_ const Camera &camera);
		/* Starts the basic section of the static geometry pipeline. */
		void BeginGeometry(void);
		/* Starts the advanced section of the static geometry pipeline. */
		void BeginAdvanced(void);
		/* Starts the second phase of the deferred rendering pipeline. */
		void BeginLight(void);
		/* End the deferred rendering pipeline. */
		void End(void);
		/* Renders the specified terrain piece to the G-Buffer. */
		void Render(_In_ const MeshCollection &meshes, const Terrain &terrain);
		/* Renders the specified model to the G-Buffer. */
		void Render(_In_ const Model &model, _In_ const Matrix &transform);
		/* Renders the specified direction light onto the scene. */
		void Render(_In_ const DirectionalLight &light);
		/* Sets the skybox to use. */
		void SetSkybox(_In_ const TextureCube &texture);
		/* Gets the amount of context switches performed since the last call. */
		_Check_return_ uint32 GetBindCount(void) const;
		/* Gets the amount of draw calls performed since the last call. */
		_Check_return_ uint32 GetDrawCount(void) const;

	private:
		DepthBuffer *depthBuffer;
		Renderpass *renderpass;
		vector<TextureInput2D*> textures;
		
		AssetFetcher *fetcher;
		GameWindow *wnd;
		GraphicsPipeline *gfxTerrain, *gfxGPassBasic, *gfxGPassAdv, *gfxLightPass, *gfxSkybox, *gfxTonePass;

		DescriptorPool *descPoolInput;
		DescriptorSetGroup *descSetInput;
		Descriptor *skybox;

		CommandBuffer *curCmd;
		const Camera *curCam;
		QueryChain *timer;
		mutable uint32 binds, draws;

		float hdrSwapchain;
		bool markNeeded, advanced, wireframe;

		void DoSkybox(void);
		void DoTonemap(void);
		void OnSwapchainRecreated(const GameWindow&, const SwapchainReCreatedEventArgs &args);
		void InitializeRenderpass(Renderpass&);
		void FinalizeRenderpass(Renderpass&);
		void CreateSizeDependentResources(void);
		void WriteDescriptors(void);
		void CreateFramebuffer(void);
		void DestroyWindowDependentResources(void);
		void DestroyPipelines(void);
		void Destroy(void);
	};
}