#pragma once
#include "DescriptorSet.h" 

namespace Pu
{
	class Renderpass;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Initializes a new instance of a descriptor pool for a specific renderpass. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ size_t maxSets);
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

		/* Gets whether the maximum amount of allocations has been reached. */
		_Check_return_ inline bool CanAllocate(void) const
		{
			return used < max;
		}

	private:
		friend class DescriptorSet;
		friend class CommandBuffer;

		const Renderpass *renderpass;
		DescriptorPoolHndl hndl;
		mutable uint32 max, used;

		void Destroy(void);
		void FreeSet(DescriptorSetHndl set) const;
	};
}