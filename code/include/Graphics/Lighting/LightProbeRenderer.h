#pragma once
#include "LightProbe.h"
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Models/Mesh.h"

namespace Pu
{
	class QueryChain;

	/* Defines an object used to render geometry to a light probe. */
	class LightProbeRenderer
	{
	public:
		/* Initializes a new instance of a light probe renderer. */
		LightProbeRenderer(_In_ AssetFetcher &loader, _In_ uint32 maxProbeCount);
		LightProbeRenderer(_In_ const LightProbeRenderer&) = delete;
		/* Move constructor. */
		LightProbeRenderer(_In_ LightProbeRenderer &&value);
		/* Releases the resources allocated by the light probe renderer. */
		~LightProbeRenderer(void)
		{
			Destroy();
		}

		_Check_return_ LightProbeRenderer& operator =(_In_ const LightProbeRenderer&) = delete;
		/* Move assignment. */
		_Check_return_ LightProbeRenderer& operator =(_In_ LightProbeRenderer &&other);

		/* Stages the light probe information to the GPU. */
		void Initialize(_In_ CommandBuffer &cmdBuffer);
		/* Starts the render process for a specific light probe. */
		void Start(_In_ LightProbe &probe, _In_ CommandBuffer &cmdBuffer) const;
		/* Renders a model to the light probe. */
		void Render(_In_ CommandBuffer &cmdBuffer, _In_ const Model &model, _In_ const Matrix &transform);
		/* Finalizes the render process for a specific light probe. */
		void End(_In_ LightProbe &probe, _In_ CommandBuffer &cmdBuffer) const;
		/* Creates a new descriptor pool for materials rendered through this light probe renderer. */
		_Check_return_ DescriptorPool* CreateDescriptorPool(_In_ uint32 maxMaterials) const;

		/* Gets whether the light probe renderer can be used. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return renderpass->IsLoaded();
		}

		/* Gets the descriptor for the diffuse texture. */
		_Check_return_ inline const Descriptor& GetDiffuseDescriptor(void) const
		{
			return renderpass->GetSubpass(0).GetDescriptor("Diffuse");
		}

		/* Gets the descriptor set layout for the materials used my the light probe renderer (set 1). */
		_Check_return_ inline const DescriptorSetLayout& GetLayout(void) const
		{
			return renderpass->GetSubpass(0).GetSetLayout(1);
		}

	private:
		friend class LightProbe;

		AssetFetcher *loader;
		uint32 maxSets;
		QueryChain *timer;

		Renderpass *renderpass;
		GraphicsPipeline *gfx;
		DescriptorPool *pool;

		void InitializeRenderpass(Renderpass&);
		void InitializePipeline(Renderpass&);
		void Destroy(void);
	};
}