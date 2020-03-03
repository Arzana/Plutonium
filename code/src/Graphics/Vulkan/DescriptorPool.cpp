#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets, std::initializer_list<std::pair<uint32, std::initializer_list<uint32>>> sets)
	: device(renderpass.device), setStride(0), allocCnt(0)
{
	vector<DescriptorPoolSize> sizes;

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

					/* 
					Add the uniform buffer descriptor size. 
					Also add it to a list of descriptors that should be written to the buffer afterwards.
					*/
					if (descriptor.GetType() == DescriptorType::UniformBuffer)
					{
						setStride = descriptor.GetAllignedOffset(setStride) + descriptor.GetSize();
						writes.emplace_back(&descriptor);
					}

					break;
				}
			}
		}
	}

	Create(sizes, maxSets);

	/* Allocate the dynamic buffer if this pool has uniform buffers. */
	if (setStride) buffer = new DynamicBuffer(*device, setStride * maxSets, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device),
	setStride(value.setStride), allocCnt(value.allocCnt), writes(std::move(value.writes))
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
		setStride = other.setStride;
		allocCnt = other.allocCnt;
		writes = std::move(other.writes);

		other.hndl = nullptr;
		other.buffer = nullptr;
	}

	return *this;
}

void Pu::DescriptorPool::Initialize(CommandBuffer & cmdBuffer, PipelineStageFlag dstStage)
{
	cmdBuffer.MemoryBarrier(*buffer, PipelineStageFlag::Transfer, dstStage, AccessFlag::UniformRead);
}

Pu::BufferHndl Pu::DescriptorPool::GetBuffer(void) const
	const DescriptorSetAllocateInfo allocInfo{ hndl, layoutHndl };
	return buffer->bufferHndl;
}

Pu::DeviceSize Pu::DescriptorPool::Alloc(DescriptorSetLayoutHndl layout, DescriptorSetHndl * result) const
{
	++allocCnt;

	/* Allocate the set from the pool, Vulkan will throw an exception if we don't have any descriptors left. */
	const DescriptorSetAllocateInfo info{ hndl, layout };
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &info, result), PFN_vkAllocateDescriptorSets);

	/* 
	We return the buffer offset if this pool allocated a buffer.
	This is simply the stride of a single descriptor set multiplies with the previous allocation count.
	*/
	if (buffer) return setStride * (allocCnt - 1);
	else return 0;
}

void Pu::DescriptorPool::Free(DescriptorSetHndl set) const
{
	--allocCnt;
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
	if (hndl)
	{
		if (allocCnt > 0) Log::Error("Destroying descriptor pool without first destroying the %u remaining sets!", allocCnt);
		device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
	}
}