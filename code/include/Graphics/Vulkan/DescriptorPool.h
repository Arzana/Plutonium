#pragma once
#include "DescriptorSet.h"
#include "Shaders/Renderpass.h"

namespace Pu
{
	class Subpass;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Initializes a new instance of a descriptor pool for a specific descriptor set. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ const Subpass &subpass, _In_ uint32 set, _In_ size_t maxSets);
		/* Initializes a new instance of a descriptor pool for a specific descriptor set used in multiple subpasses. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ uint32 set, _In_ size_t maxSets);
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

		/* Gets the subpass associated with this descriptor pool (only usable if this descriptor pool was created using 1 subpass). */
		_Check_return_ inline const Subpass& GetSubpass(void) const
		{
			return *subpass;
		}

		/* Gets the subpass at the specified index. */
		_Check_return_ inline const Subpass& GetSubpass(_In_ size_t index) const
		{
			return renderpass->GetSubpass(index);
		}

	private:
		friend class DescriptorSet;
		friend class CommandBuffer;

		const Subpass *subpass;
		const Renderpass *renderpass;
		LogicalDevice *device;

		DescriptorPoolHndl hndl;
		PipelineLayoutHndl pipelineLayout;
		DescriptorSetLayoutHndl descriptorLayout;
		mutable uint32 max, used, set;

		void Create(vector<DescriptorPoolSize> &sizes);
		void Destroy(void);
		void FreeSet(DescriptorSetHndl setHndl) const;
	};
}