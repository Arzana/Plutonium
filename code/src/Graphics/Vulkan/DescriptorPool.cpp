#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, const Pipeline & pipeline, uint32 maxSets)
	: device(renderpass.device), renderpass(&renderpass), pipeline(&pipeline),
	OnStage("DescriptorPoolOnStage"), setStride(0), allocCnt(0), maxSets(maxSets), buffer(nullptr)
{}

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, const Pipeline & pipeline, uint32 maxSets, uint32 subpass, uint32 set)
	: DescriptorPool(renderpass, pipeline, maxSets)
{
	AddSets(subpass, { set });
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: hndl(value.hndl), buffer(value.buffer), device(value.device), maxSets(value.maxSets),
	setStride(value.setStride), allocCnt(value.allocCnt), writes(std::move(value.writes)),
	sizes(std::move(value.sizes)), OnStage(std::move(value.OnStage)),
	renderpass(value.renderpass), pipeline(value.pipeline)
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
		writes = std::move(other.writes);
		sizes = std::move(other.sizes);
		renderpass = other.renderpass;
		pipeline = other.pipeline;
		OnStage = std::move(other.OnStage);

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

	/* Get the descriptors that are part of the specified subpass and sets. */
	for (const Descriptor &descriptor : renderpass->GetSubpass(subpass).descriptors)
	{
		for (uint32 set : sets)
		{
			if (descriptor.GetSet() != set) continue;
			const DescriptorType type = descriptor.GetType();

			/* Either add the descriptor to the size list or increase the specific descriptor count by one. */
			decltype(sizes)::iterator it = sizes.iteratorOf([type](const DescriptorPoolSize &cur) { return cur.Type == type; });
			if (it != sizes.end()) ++it->DescriptorCount;
			else sizes.emplace_back(descriptor.GetType(), 1);

			/* 
			The stride needs to be increased if we found a uniform buffer descriptor. 
			We also need to add it to a list of descriptors that need initialization.
			*/
			if (type == DescriptorType::UniformBuffer)
			{
				setStride = descriptor.GetAllignedOffset(setStride) + descriptor.GetSize();
				writes.emplace_back(&descriptor);
			}

			break;
		}
	}
}

void Pu::DescriptorPool::Update(CommandBuffer & cmdBuffer, PipelineStageFlag dstStage)
{
	/* Create the pool if it hasn't been created yet. */
	if (!hndl) Create();

	/* We just need to create and initialize the underlying buffer if the set actually has uniform buffers. */
	if (setStride && !buffer)
	{
		buffer = new DynamicBuffer(*device, setStride * maxSets, BufferUsageFlag::TransferDst | BufferUsageFlag::UniformBuffer);
		cmdBuffer.MemoryBarrier(*buffer, PipelineStageFlag::Transfer, dstStage, AccessFlag::UniformRead);
	}
	else
	{
		/* Start by staging the memory to the buffer. */
		buffer->BeginMemoryTransfer();
		OnStage.Post(*this, reinterpret_cast<byte*>(buffer->GetHostMemory()));
		buffer->EndMemoryTransfer();

		/* Update the contents of the dynamic buffer. */
		buffer->Update(cmdBuffer);
	}
}

Pu::BufferHndl Pu::DescriptorPool::GetBuffer(void) const
{
	return buffer->bufferHndl;
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
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl)
	{
		if (allocCnt > 0) Log::Error("Destroying descriptor pool without first destroying the %u remaining sets!", allocCnt);
		device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
	}
}