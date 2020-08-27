#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass)
	: device(renderpass.device), renderpass(&renderpass), firstUpdate(true), stride(0),
	OnStage("DescriptorPoolOnStage", true), buffer(nullptr), maxSets(0)
{}

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, uint32 maxSets, uint32 subpass, uint32 set)
	: DescriptorPool(renderpass)
{
	AddSet(subpass, set, maxSets);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device),
	stride(value.stride), sets(std::move(value.sets)), maxSets(value.maxSets),
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

void Pu::DescriptorPool::AddSet(uint32 subpass, uint32 set, uint32 max)
{
	/* Check for invalid use on debug mode. */
#ifdef _DEBUG
	if (hndl) Log::Fatal("Cannot add set to descriptor pool (%x) after it has been initialized!", hndl);

	if (sets.contains([subpass, set](const SetInfo &cur) { return cur.Id == MakeId(subpass, set); }))
	{
		Log::Fatal("Set %u from subpass %u has already been added to this descriptor pool!", set, subpass);
	}
#endif

	/* Handle uniform buffer sets. */
	const DescriptorSetLayout &layout = renderpass->GetSubpass(subpass).GetSetLayout(set);
	if (layout.HasUniformBufferMemory())
	{
		/* Calculate the end of the last set. */
		const DeviceSize end = sets.empty() ? 0 : sets.back().Offset + sets.back().GetMaxSets() * stride;

		/*
		Add the current set to the list and set the last stride.
		stride is always alligned so the end pointer will also be alligned.
		*/
		sets.emplace_back(subpass, set, max, end);
		stride = layout.GetAllignedStride();
	}

	/* Add all the descriptors to the allocation list. */
	for (const Descriptor *descriptor : layout.descriptors)
	{
		const DescriptorType type = descriptor->GetType();

		/* We either increase the count or add a new descriptor with count 1. */
		decltype(sizes)::iterator it = sizes.iteratorOf([type](const DescriptorPoolSize &cur) { return cur.Type == type; });
		if (it != sizes.end()) it->DescriptorCount += max;
		else sizes.emplace_back(type, max);
	}

	/* Increase the counter for the maximum number of descriptor set that can be allocated. */
	maxSets += max;
}

void Pu::DescriptorPool::Update(CommandBuffer & cmdBuffer, PipelineStageFlag dstStage)
{
	/* Create the pool if it hasn't been created yet. */
	if (!hndl) Create();

	if (buffer)
	{
		/* Start by staging the memory to the buffer. */
		buffer->BeginMemoryTransfer();
		OnStage.Post(*this, reinterpret_cast<byte*>(buffer->GetHostMemory()));
		buffer->EndMemoryTransfer();

		/* Update the contents of the dynamic buffer. */
		buffer->Update(cmdBuffer);

		/* We need to move the buffer to uniform read mode once. */
		if (firstUpdate)
		{
			firstUpdate = false;
			cmdBuffer.MemoryBarrier(*buffer, PipelineStageFlag::Transfer, dstStage, AccessFlag::UniformRead);
		}
	}
}

void Pu::DescriptorPool::Reset(void)
{
	VK_VALIDATE(device->vkResetDescriptorPool(device->hndl, hndl, 0), PFN_vkResetDescriptorPool);
}

Pu::DeviceSize Pu::DescriptorPool::Alloc(uint32 subpass, const DescriptorSetLayout & layout, DescriptorSetHndl * result)
{
	/* Lazily create if needed. */
	lock.lock();
	if (!hndl) Create();

	/* Allocate the set from the pool, Vulkan will throw an exception if we don't have any descriptors left. */
	const DescriptorSetAllocateInfo info{ hndl, layout.hndl };
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &info, result), PFN_vkAllocateDescriptorSets);

	/* Return the buffer offset if this set is a uniform buffer. */
	if (buffer)
	{
		/* Get the base offset of the set in the buffer. */
		const uint64 id = MakeId(subpass, layout.set);
		decltype(sets)::iterator it = sets.iteratorOf([id](const SetInfo &cur) { return cur.Id == id; });
		if (it != sets.end())
		{
			/* Find an unused space in the buffer. */
			for (uint32 i = 0; i < it->spaces.size(); i++)
			{
				if (!it->spaces[i])
				{
					/* Mark the space as used and return the offset of it. */
					it->spaces[i] = *result;
					lock.unlock();

					return it->Offset + layout.GetAllignedStride() * i;
				}
			}

			ThrowInvalidAlloc(subpass, layout.set, "max set allocations reached");
		}
		else ThrowInvalidAlloc(subpass, layout.set, "combination wasn't specified during creation");
	}

	/* No offset is needed as the descriptor set doesn't have buffer memory. */
	lock.unlock();
	return 0;
}

void Pu::DescriptorPool::Free(DescriptorSetHndl set)
{
	lock.lock();

	if (buffer)
	{
		for (SetInfo &info : sets)
		{
			for (DescriptorSetHndl &space : info.spaces)
			{
				if (space == set)
				{
					/* Goto is used to break out of nested for loop. */
					space = nullptr;
					goto FreeDescriptorSet;
				}
			}
		}
	}

FreeDescriptorSet:
	VK_VALIDATE(device->vkFreeDescriptorSets(device->hndl, hndl, 1, &set), PFN_vkFreeDescriptorSets);
	lock.unlock();
}

void Pu::DescriptorPool::Create(void)
{
	const DescriptorPoolCreateInfo info{ maxSets, sizes };
	VK_VALIDATE(device->vkCreateDescriptorPool(device->hndl, &info, nullptr, &hndl), PFN_vkCreateDescriptorPool);

	/* The buffer is needed for the set to do a write when it allocates. */
	if (sets.size())
	{
		/* We must allign the final set stride to the physical device allignment, otherwise multiple sets will not start at proper allignment. */
		const DeviceSize size = device->GetPhysicalDevice().GetUniformBufferOffsetAllignment(sets.back().Offset + stride * sets.back().GetMaxSets());
		buffer = new DynamicBuffer(*device, size, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
		buffer->SetDebugName("Uniform Buffer");
	}
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl)
	{
		device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
		delete buffer;
	}
}

void Pu::DescriptorPool::ThrowInvalidAlloc(uint32 subpass, uint32 set, const char * reason)
{
	Log::Fatal("Cannot allocate set %u (subpass %u) from descriptor pool (%s)!", set, subpass, reason);
}

Pu::uint64 Pu::DescriptorPool::MakeId(uint32 subpass, uint32 set)
{
	constexpr Pu::uint64 bits_per_uint32 = sizeof(Pu::uint32) << 3;
	return (static_cast<Pu::uint64>(subpass) << bits_per_uint32) | set;
}

Pu::DescriptorPool::SetInfo::SetInfo(uint32 subpass, uint32 set, uint32 max, DeviceSize offset)
	: Id(MakeId(subpass, set)), Offset(offset)
{
	spaces.resize(max, nullptr);
}