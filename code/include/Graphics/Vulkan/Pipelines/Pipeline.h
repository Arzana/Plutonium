#pragma once
#include "PipelineLayout.h"

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
			Destroy();
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

		/* Gets the logical device on which this pipeline lives. */
		_Check_return_ inline LogicalDevice& GetDevice(void) const
		{
			return *layout.device;
		}

		/* Gets the layout of this pipeline. */
		_Check_return_ inline const PipelineLayout& GetLayout(void) const
		{
			return layout;
		}

	protected:
		PipelineHndl Hndl;

		/* Initializes a new instance of a Vulkan pipeline. */
		Pipeline(_In_ LogicalDevice &device, _In_ const Subpass &subpass);

		/* Gets the handle of the pipeline layout. */
		_Check_return_ inline PipelineLayoutHndl GetLayoutHndl(void) const
		{
			return layout.hndl;
		}

		/* Gets the shader stages used by the pipeline. */
		_Check_return_ inline const vector<PipelineShaderStageCreateInfo>& GetShaderStages(void) const
		{
			return shaderStages;
		}

		/* Releases the pipeline handle. */
		void Destroy(void);

	private:
		friend class CommandBuffer;
		friend class DescriptorPool;

		PipelineLayout layout;
		vector<PipelineShaderStageCreateInfo> shaderStages;
	};
}