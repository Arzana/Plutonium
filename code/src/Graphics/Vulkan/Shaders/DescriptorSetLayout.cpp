#include "Graphics/Vulkan/Shaders/DescriptorSetLayout.h"

Pu::DescriptorSetLayout::DescriptorSetLayout(LogicalDevice & device, const vector<const Descriptor*>& descriptors)
	: descriptors(descriptors), device(&device)
{
	/* Check for invalid usage on debug mode. */
#ifdef _DEBUG
	if (descriptors.empty()) Log::Fatal("Descriptor set layout needs at least 1 descriptor!");
#endif

	/* Set the set that we'll handle and pre-allocate the handle data. */
	set = descriptors.front()->GetSet();
	vector<DescriptorSetLayoutBinding> bindings;
	bindings.reserve(descriptors.size());

	/* Set the layout bindings (and stride if needed). */
	DeviceSize offset = 0;
	for (const Descriptor *descriptor : descriptors)
	{
		const uint32 binding = descriptor->GetBinding();

		/* Check for invalid usage on debug mode. */
#ifdef _DEBUG
		if (descriptor->GetSet() != set)
		{
			Log::Fatal("Descriptor set mismatch (descriptor '%s' is in set %u, expected set %u)!", descriptor->GetInfo().Name.c_str(), descriptor->GetSet(), set);
		}
#endif

		/* Only add a new binding if it was not yet defined. */
		if (bindings.contains([binding](const DescriptorSetLayoutBinding &cur) { return cur.Binding == binding; }))
		{
			bindings.emplace_back(descriptor->layoutBinding);
		}

		/* Increase the stride if this is descriptor is part of a uniform block. */
		if (descriptor->GetType() == DescriptorType::UniformBuffer)
		{
			const DeviceSize size = descriptor->GetSize();

			/* Add the size to the already existing binding if it was already added. */
			std::map<uint32, Range>::iterator it = ranges.find(binding);
			if (it != ranges.end()) it->second.second += size;
			else
			{
				/*
				The binding is not yet found, add a new binding range to the list.
				We need to take external offset into account when adding a new binding.
				*/
				offset = descriptor->GetAllignedOffset(offset);
				ranges.emplace(binding, std::make_pair(offset, size));
			}

			offset += size;
		}
	}

	/* Create the handle to the set layout. */
	const DescriptorSetLayoutCreateInfo createInfo{ bindings };
	VK_VALIDATE(device.vkCreateDescriptorSetLayout(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateDescriptorSetLayout);
}

Pu::DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout && value)
	: hndl(value.hndl), device(value.device), descriptors(std::move(value.descriptors)),
	ranges(std::move(value.ranges)), set(value.set)
{
	value.hndl = nullptr;
}

Pu::DescriptorSetLayout & Pu::DescriptorSetLayout::operator=(DescriptorSetLayout && other)
{
	if (this != &other)
	{
		Destroy();

		hndl = other.hndl;
		device = other.device;
		descriptors = std::move(other.descriptors);
		ranges = std::move(other.ranges);
		set = other.set;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::DeviceSize Pu::DescriptorSetLayout::GetStride(void) const
{
	DeviceSize stride = 0;

	/* Just loop through the ranges to get the final range's end position. */
	for (const auto[binding, range] : ranges)
	{
		const DeviceSize end = range.first + range.second;
		if (end > stride) stride = end;
	}

	return stride;
}

void Pu::DescriptorSetLayout::Destroy(void)
{
	if (hndl) device->vkDestroyDescriptorSetLayout(device->hndl, hndl, nullptr);
}