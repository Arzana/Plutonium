#pragma once
#include "Descriptor.h"
#include "Graphics/Vulkan/LogicalDevice.h"

namespace Pu
{
	/* Defines the properties of a descriptor set layout. */
	class DescriptorSetLayout
	{
	public:
		/* Initializes a new instance of a descriptor set layout for the specified descriptors. */
		DescriptorSetLayout(_In_ LogicalDevice &device, _In_ const vector<const Descriptor*> &descriptors);
		DescriptorSetLayout(_In_ const DescriptorSetLayout&) = delete;
		/* Move constructor. */
		DescriptorSetLayout(_In_ DescriptorSetLayout &&value);
		/* Releases the resources allocated by the descriptor set layout. */
		~DescriptorSetLayout(void)
		{
			Destroy();
		}

		_Check_return_ DescriptorSetLayout& operator =(_In_ const DescriptorSetLayout&) = delete;
		/* Move assignment. */
		_Check_return_ DescriptorSetLayout& operator =(_In_ DescriptorSetLayout &&other);

		/* Gets the stride of the uniform block in the set (if this is not a uniform block, returns 0). */
		_Check_return_ DeviceSize GetStride(void) const;
		/* Gets the stride of the uniform block in the set (alligned to the physical device minimum). */
		_Check_return_ DeviceSize GetAllignedStride(void) const;

		/* Gets the set number for this descriptor set layout. */
		_Check_return_ inline uint32 GetSet(void) const
		{
			return set;
		}

		/* Gets whether this descriptor set layout contains uniform buffer memory. */
		_Check_return_ inline bool HasUniformBufferMemory(void) const
		{
			return ranges.size();
		}

	private:
		friend class Pipeline;
		friend class DescriptorPool;
		friend class DescriptorSet;

		using Range = std::pair<DeviceSize, DeviceSize>;

		DescriptorSetLayoutHndl hndl;
		LogicalDevice *device;

		vector<const Descriptor*> descriptors;
		std::map<uint32, Range> ranges;
		uint32 set;

		void Destroy(void);
	};
}