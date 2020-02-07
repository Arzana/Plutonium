#pragma once
#include "Graphics/Vulkan/Shaders/Subpass.h"

namespace Pu
{
	/* Defines the base class for a Vulkan graphics or compute pipeline. */
	class Pipeline
	{
	public:
		Pipeline(_In_ const Pipeline&) = delete;
		/* Move constructor. */
		Pipeline(_In_ Pipeline &&value);
		/* Releases the resources allocated by the pipeline. */
		virtual ~Pipeline(void)
		{
			FullDestroy();
		}

		_Check_return_ Pipeline& operator =(_In_ const Pipeline&) = delete;
		/* Move assignment. */
		_Check_return_ Pipeline& operator =(_In_ Pipeline &&other);

		/* Finalizes the pipeline, creating the underlying Vulkan resource. */
		virtual void Finalize(void) = 0;

		/* Gets whether this pipeline can be bound. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return Hndl;
		}

	protected:
		/* The raw Vulkan handle to the pipeline. */
		PipelineHndl Hndl;
		/* The raw Vulkan handle to the pipeline layout. */
		PipelineLayoutHndl LayoutHndl;
		/* The logical device on which this pipeline was created. */
		LogicalDevice *Device;

		/* Initializes a new instance of a Vulkan pipeline. */
		Pipeline(_In_ LogicalDevice &device, _In_ const Subpass &subpass);

		/* Gets the shader stages used by the pipeline. */
		_Check_return_ inline const vector<PipelineShaderStageCreateInfo>& GetShaderStages(void) const
		{
			return shaderStages;
		}

		/* Releases the pipeline. */
		void Destroy(void);

	private:
		friend class CommandBuffer;
		friend class DescriptorSet;

		vector<PipelineShaderStageCreateInfo> shaderStages;
		vector<DescriptorSetLayoutHndl> setHndls;

		void CreateDescriptorSetLayouts(const Subpass &subpass);
		void CreatePipelineLayout(const Subpass &subpass);
		void FullDestroy(void);
	};
}