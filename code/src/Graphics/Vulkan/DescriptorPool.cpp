#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets)
	: device(renderpass.device), renderpass(&renderpass), firstUpdate(true),
	OnStage("DescriptorPoolOnStage"), setStride(0), allocCnt(0), maxSets(maxSets), buffer(nullptr)
{}

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets, uint32 subpass, uint32 set)
	: DescriptorPool(renderpass, maxSets)
{
	AddSets(subpass, { set });
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device), maxSets(value.maxSets),
	setStride(value.setStride), allocCnt(value.allocCnt),
	sizes(std::move(value.sizes)), OnStage(std::move(value.OnStage)),
	renderpass(value.renderpass), firstUpdate(value.firstUpdate)
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
		maxSets = other.maxSets;
		sizes = std::move(other.sizes);
		renderpass = other.renderpass;
		OnStage = std::move(other.OnStage);
		firstUpdate = other.firstUpdate;

		other.hndl = nullptr;
		other.buffer = nullptr;
	}

	return *this;
}

void Pu::DescriptorPool::AddSets(uint32 subpass, std::initializer_list<uint32> sets)
{
	/* Check if the pool hasn't been initialized yet on debug mode. */
#ifdef _DEBUG
	if (hndl)
	{
		Log::Error("Cannot add sets to descriptor pool (%x) after it has been initialized!", hndl);
		return;
	}
#endif

	for (uint32 set : sets)
	{
		/* Get the layout of this set and increase the total stride. */
		const DescriptorSetLayout &layout = renderpass->GetSubpass(subpass).GetSetLayout(set);

		/* Only increase the set stride if the set actually has a uniform buffer. */
		if (layout.HasUniformBufferMemory())
		{
			setStride = device->GetPhysicalDevice().GetUniformBufferOffsetAllignment(setStride) + layout.GetStride();
		}

		/* Add all the descriptors to the allocation list. */
		for (const Descriptor *descriptor : layout.descriptors)
		{
			const DescriptorType type = descriptor->GetType();

			/* Add the descriptor to out allocation list. */
			decltype(sizes)::iterator it = sizes.iteratorOf([type](const DescriptorPoolSize &cur) {return cur.Type == type; });
			if (it != sizes.end()) ++it->DescriptorCount;
			else sizes.emplace_back(type, 1);
		}
	}
}

void Pu::DescriptorPool::Update(CommandBuffer & cmdBuffer, PipelineStageFlag dstStage)
{
	/* Create the pool if it hasn't been created yet. */
	if (!hndl) Create();

	if (buffer)
	{
		/* We need to move the buffer to uniform read mode once. */
		if (firstUpdate)
		{
			firstUpdate = false;
			cmdBuffer.MemoryBarrier(*buffer, PipelineStageFlag::Transfer, dstStage, AccessFlag::UniformRead);
		}

		/* Start by staging the memory to the buffer. */
		buffer->BeginMemoryTransfer();
		OnStage.Post(*this, reinterpret_cast<byte*>(buffer->GetHostMemory()));
		buffer->EndMemoryTransfer();

		/* Update the contents of the dynamic buffer. */
		buffer->Update(cmdBuffer);
	}
}

Pu::DeviceSize Pu::DescriptorPool::Alloc(DescriptorSetLayoutHndl layout, DescriptorSetHndl * result)
{
	if (++allocCnt > maxSets) Log::Fatal("Attempting to allocate descriptor set outside of pool range!");
	if (!hndl) Create();

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

void Pu::DescriptorPool::Free(DescriptorSetHndl set)
{
	--allocCnt;
	VK_VALIDATE(device->vkFreeDescriptorSets(device->hndl, hndl, 1, &set), PFN_vkFreeDescriptorSets);
}

void Pu::DescriptorPool::Create(void)
{
	/* The amount of descriptors is global, so we need to multiply our origional count by the maximum number of sets. */
	for (DescriptorPoolSize &size : sizes) size.DescriptorCount *= maxSets;

	const DescriptorPoolCreateInfo info{ maxSets, sizes };
	VK_VALIDATE(device->vkCreateDescriptorPool(device->hndl, &info, nullptr, &hndl), PFN_vkCreateDescriptorPool);

	/* The buffer is needed for the set to do a write when it allocates. */
	if (setStride) buffer = new DynamicBuffer(*device, setStride * maxSets, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl)
	{
		if (allocCnt > 0) Log::Error("Destroying descriptor pool without first destroying the %u remaining sets!", allocCnt);
		device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
		delete buffer;
	}
}