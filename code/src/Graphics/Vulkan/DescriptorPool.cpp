#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets, std::initializer_list<std::pair<uint32, std::initializer_list<uint32>>> sets)
	: device(renderpass.device)
{
	vector<DescriptorPoolSize> sizes;
	DeviceSize bufferSize = 0;

	/* Loop through all the subpasses and descriptors to get the sets that should contribute to the pool. */
	for (const auto &[subpass, setIdxs] : sets)
	{
		for (const Descriptor &descriptor : renderpass.GetSubpass(subpass).descriptors)
		{
			for (uint32 set : setIdxs)
			{
				/* Only add the descriptor if it's part of the subpass sets. */
				if (descriptor.GetSet() == set)
				{
					decltype(sizes)::iterator it = sizes.iteratorOf([&descriptor](const DescriptorPoolSize &cur) { return cur.Type == descriptor.GetType(); });
					if (it != sizes.end()) ++it->DescriptorCount;
					else sizes.emplace_back(descriptor.GetType(), 1);

					/* Add the uniform buffer descriptor size. */
					if (descriptor.GetType() == DescriptorType::UniformBuffer)
					{
						bufferSize = descriptor.GetAllignedOffset(bufferSize) + descriptor.GetSize();
					}

					break;
				}
			}
		}
	}

	Create(sizes, maxSets);

	/* Allocate the dynamic buffer if this pool has uniform buffers. */
	if (bufferSize) buffer = new DynamicBuffer(*device, bufferSize, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device)
{
	value.hndl = nullptr;
	value.buffer = nullptr;
}

Pu::DescriptorPool & Pu::DescriptorPool::operator=(DescriptorPool && other)
{
	if (this != &other)
	{
		Destroy();
		
		hndl = other.hndl;
		buffer = other.buffer;
		device = other.device;

		other.hndl = nullptr;
		other.buffer = nullptr;
	}

	return *this;
}

void Pu::DescriptorPool::Alloc(DescriptorSetLayoutHndl layout, DescriptorSetHndl * result) const
{
	const DescriptorSetAllocateInfo info{ hndl, layout };
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &info, result), PFN_vkAllocateDescriptorSets);
}

void Pu::DescriptorPool::Free(DescriptorSetHndl set) const
{
	VK_VALIDATE(device->vkFreeDescriptorSets(device->hndl, hndl, 1, &set), PFN_vkFreeDescriptorSets);
}

void Pu::DescriptorPool::Create(vector<DescriptorPoolSize>& sizes, uint32 maxSets)
{
	/* The amount of descriptors is global, so we need to multiply our origional count by the maximum number of sets. */
	for (DescriptorPoolSize &size : sizes) size.DescriptorCount *= maxSets;

	const DescriptorPoolCreateInfo info{ maxSets, sizes };
	VK_VALIDATE(device->vkCreateDescriptorPool(device->hndl, &info, nullptr, &hndl), PFN_vkCreateDescriptorPool);
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl) device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
}