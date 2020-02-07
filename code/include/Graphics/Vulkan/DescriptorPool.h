#pragma once
#include "Shaders/Renderpass.h"

namespace Pu
{
	class DynamicBuffer;

	/* Defines an allocation pool for Vulkan descriptors. */
	class DescriptorPool
	{
	public:
		/* Creates a descriptor pool for the specific descriptor sets in the specific subpasses. */
		DescriptorPool(_In_ const Renderpass &renderpass, _In_ uint32 maxSets, _In_ std::initializer_list<std::pair<uint32, std::initializer_list<uint32>>> sets);
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

	private:
		friend class DescriptorSet;

		DescriptorPoolHndl hndl;
		DynamicBuffer *buffer;
		LogicalDevice *device;

		void Alloc(DescriptorSetLayoutHndl layout, DescriptorSetHndl *result) const;
		void Free(DescriptorSetHndl set) const;
		void Create(vector<DescriptorPoolSize> &sizes, uint32 maxSets);
		void Destroy(void);
	};
}