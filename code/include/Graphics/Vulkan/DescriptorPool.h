#pragma once
#include "DescriptorSet.h" 

namespace Pu
{
	class Renderpass;
	class Subpass;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Initializes a new instance of a descriptor pool for a specific descriptor set. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ const Subpass &subpass, _In_ uint32 set, _In_ size_t maxSets);
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
		_Check_return_ DescriptorSet Allocate(void) const;
		/* Deallocates the specified descriptor set from this pool. */
		void DeAllocate(_In_ DescriptorSet &set);

		/* Gets the set that this descriptor pool allocates. */
		_Check_return_ inline uint32 GetSet(void) const
		{
			return set;
		}

		/* Gets whether the maximum amount of allocations has been reached. */
		_Check_return_ inline bool CanAllocate(void) const
		{
			return used < max;
		}

		/* Gets the subpass associated with this descriptor pool. */
		_Check_return_ inline const Subpass& GetSubpass(void) const
		{
			return *subpass;
		}

	private:
		friend class DescriptorSet;
		friend class CommandBuffer;

		const Subpass *subpass;
		LogicalDevice *device;

		DescriptorPoolHndl hndl;
		PipelineLayoutHndl pipelineLayout;
		DescriptorSetLayoutHndl descriptorLayout;
		mutable uint32 max, used, set;

		void Destroy(void);
		void FreeSet(DescriptorSetHndl setHndl) const;
	};
}