#pragma once
#include "Graphics/Vulkan/Shaders/Subpass.h"

namespace Pu
{
	/* Defines a pipeline layout for a specific subpass (or compute pipeline). */
	class PipelineLayout
	{
	public:
		/* Initializes a new instance of a pipeline layout for a specific subpass. */
		PipelineLayout(_In_ LogicalDevice &device, _In_ const Subpass &subpass);
		PipelineLayout(_In_ const PipelineLayout&) = delete;
		/* Move constructor. */
		PipelineLayout(_In_ PipelineLayout &&value);
		/* Releases the resources allocated by the pipeline layout. */
		~PipelineLayout(void)
		{
			Destroy();
		}

		_Check_return_ PipelineLayout& operator =(_In_ const PipelineLayout&) = delete;
		/* Move assignment. */
		_Check_return_ PipelineLayout& operator =(_In_ PipelineLayout &&other);

	private:
		friend class CommandBuffer;
		friend class DescriptorPool;
		friend class GraphicsPipeline;

		PipelineLayoutHndl hndl;
		vector<DescriptorSetLayoutHndl> setHndls;
		LogicalDevice *device;

		void Destroy(void);
		void CreateDescriptorSetLayouts(const Subpass &subpass);
		void CreatePipelineLayout(const Subpass &subpass);
	};
}