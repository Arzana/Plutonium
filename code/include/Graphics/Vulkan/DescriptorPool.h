#pragma once
#include "LogicalDevice.h"
#include "DescriptorSet.h"

namespace Pu
{
	class GraphicsPipeline;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		DescriptorPool(_In_ const DescriptorPool&) = delete;
		/* Move constructor. */
		DescriptorPool(_In_ DescriptorPool &&value);
		/* Destroys the descriptor pool. */
		~DescriptorPool(void)
		{
			Destroy();
		}

		_Check_return_ DescriptorPool& operator =(_In_ const DescriptorPool&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorPool& operator =(_In_ DescriptorPool &&other);

		/* Allocates a new descriptor set from this pool. */
		_Check_return_ DescriptorSet Allocate(_In_ uint32 set) const;

	private:
		friend class DescriptorSet;
		friend class CommandBuffer;
		friend class GraphicsPipeline;

		GraphicsPipeline *parent;
		LogicalDevice *device;
		DescriptorPoolHndl hndl;

		DescriptorPool(_In_ GraphicsPipeline &parent, _In_ size_t maxSets);

		void Destroy(void);
		void FreeSet(DescriptorSetHndl set) const;
	};
}