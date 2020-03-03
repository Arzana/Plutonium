#pragma once
#include "Shaders/Renderpass.h"
#include "Pipelines/Pipeline.h"

namespace Pu
{
	class DynamicBuffer;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Occurs when the descriptor pool is staging its memory. */
		EventBus<DescriptorPool, byte*> OnStage;

		/* Initializes a new instance of a descriptor pool. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ const Pipeline &pipeline, _In_ uint32 maxSets);
		/* Creates a descriptor pool for the specific descriptor set in the specific subpasses. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ const Pipeline &pipeline, _In_ uint32 maxSets, _In_ uint32 subpass, _In_ uint32 set);
		DescriptorPool(_In_ const DescriptorPool &value) = delete;
		/* Move constructor. */
		DescriptorPool(_In_ DescriptorPool &&value);
		/* Releases the resources allocated by the descriptor pool. */
		~DescriptorPool(void)
		{
			Destroy();
		}

		_Check_return_ DescriptorPool& operator =(_In_ const DescriptorPool&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorPool& operator =(_In_  DescriptorPool &&other);

		/* Adds several descriptor sets in the specified subpass to the pool. */
		void AddSets(_In_ uint32 subpass, _In_ std::initializer_list<uint32> sets);
		/* Updates the descriptor pool, staging all the descriptors to the GPU. */
		void Update(_In_ CommandBuffer &cmdBuffer, _In_ PipelineStageFlag dstStage);

	private:
		friend class DescriptorSet;
		friend class UniformBlock;

		DescriptorPoolHndl hndl;
		DynamicBuffer *buffer;
		LogicalDevice *device;
		const Renderpass *renderpass;
		const Pipeline *pipeline;

		DeviceSize setStride;
		uint32 maxSets, allocCnt;
		bool firstUpdate;

		vector<const Descriptor*> writes;
		vector<DescriptorPoolSize> sizes;

		DeviceSize Alloc(DescriptorSetLayoutHndl layout, DescriptorSetHndl *result);
		void Free(DescriptorSetHndl set);
		void Create(void);
		void Destroy(void);
	};
}