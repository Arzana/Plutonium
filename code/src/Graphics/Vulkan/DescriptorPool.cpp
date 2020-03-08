#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

static Pu::uint64 make_id(Pu::uint32 subpass, Pu::uint32 set)
{
	constexpr Pu::uint64 bits_per_uint32 = sizeof(Pu::uint32) << 3;
	return (static_cast<Pu::uint64>(subpass) << bits_per_uint32) | set;
}

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets)
	: device(renderpass.device), renderpass(&renderpass), firstUpdate(true), stride(0),
	OnStage("DescriptorPoolOnStage"), allocCnt(0), maxSets(maxSets), buffer(nullptr)
{}

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets, uint32 subpass, uint32 set)
	: DescriptorPool(renderpass, maxSets)
{
	AddSet(subpass, set);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device), maxSets(value.maxSets),
	allocCnt(value.allocCnt), stride(value.stride), sets(std::move(value.sets)),
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
		stride = other.stride;
		allocCnt = other.allocCnt;
		maxSets = other.maxSets;
		sizes = std::move(other.sizes);
		renderpass = other.renderpass;
		OnStage = std::move(other.OnStage);
		firstUpdate = other.firstUpdate;
		sets = std::move(other.sets);

		other.hndl = nullptr;
		other.buffer = nullptr;
	}

	return *this;
}

void Pu::DescriptorPool::AddSet(uint32 subpass, uint32 set)
{
	const uint64 id = make_id(subpass, set);
	const DescriptorSetLayout &layout = renderpass->GetSubpass(subpass).GetSetLayout(set);

	/* Check for invalid use on debug mode. */
#ifdef _DEBUG
	if (hndl) Log::Fatal("Cannot add set to descriptor pool (%x) after it has been initialized!", hndl);

	if (sets.contains([id](const SetInfo &cur) { return cur.first == id; }))
	{
		Log::Fatal("Set %u from subpass %u has already been added to this descriptor pool!", set, subpass);
	}
#endif

	/* Handle uniform buffer sets. */
	if (layout.HasUniformBufferMemory())
	{
		/* We use the allocation stride temporarily to store stride of the last added set. */
		const DeviceSize end = sets.empty() ? 0 : sets.back().second + maxSets * stride;

		/* 
		Add the current set to the list and set the last stride. 
		stride is always alligned so the end pointer will also be alligned.
		*/
		sets.emplace_back(id, end);
		stride = layout.GetAllignedStride();
	}

	/* Add all the descriptors to the allocation list. */
	for (const Descriptor *descriptor : layout.descriptors)
	{
		const DescriptorType type = descriptor->GetType();

		/* We either increase the count or add a new descriptor with count 1. */
		decltype(sizes)::iterator it = sizes.iteratorOf([type](const DescriptorPoolSize &cur) { return cur.Type == type; });
		if (it != sizes.end()) ++it->DescriptorCount;
		else sizes.emplace_back(type, 1);
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

Pu::DeviceSize Pu::DescriptorPool::Alloc(uint32 subpass, const DescriptorSetLayout & layout, DescriptorSetHndl * result)
{
	/* Handle overflows and lazily create if needed. */
	if (++allocCnt > maxSets) Log::Fatal("Attempting to allocate descriptor set outside of pool range!");
	if (!hndl) Create();

	/* Allocate the set from the pool, Vulkan will throw an exception if we don't have any descriptors left. */
	const DescriptorSetAllocateInfo info{ hndl, layout.hndl };
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &info, result), PFN_vkAllocateDescriptorSets);

	/* Return the buffer offset if this set is a uniform buffer. */
	if (buffer)
	{
		/* Get the base offset of the set in the buffer. */
		const uint64 id = make_id(subpass, layout.set);
		const DeviceSize offset = sets.iteratorOf([id](const SetInfo &cur) { return cur.first == id; })->second;
		return offset + layout.GetAllignedStride() * (allocCnt - 1);
	}

	return 0;
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
	if (sets.size())
	{
		/* We must allign the final set stride to the physical device allignment, otherwise multiple sets will not start at proper allignment. */
		const DeviceSize size = device->GetPhysicalDevice().GetUniformBufferOffsetAllignment(sets.back().second + stride * maxSets);
		buffer = new DynamicBuffer(*device, size, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
		buffer->SetDebugName("Uniform Buffer");
	}
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