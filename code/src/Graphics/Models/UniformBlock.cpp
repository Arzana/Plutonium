#include "Graphics/Models/UniformBlock.h"

Pu::UniformBlock::UniformBlock(DescriptorPool & pool, bool startDirty)
	: DescriptorSet(std::move(pool.Allocate())), IsDirty(startDirty), firstUpdate(true)
{
	vector<uint32> bindings;
	size_t size = 0;

	/* Calculate the size of the uniform buffer based on the descriptors. */
	for (const Descriptor &descriptor : pool.GetSubpass().descriptors)
	{
		/* Only gets the uniform buffer descriptors within the set. */
		if (descriptor.GetSet() != pool.GetSet()) continue;
		if (descriptor.GetType() != DescriptorType::UniformBuffer) continue;

		descriptors.emplace_back(&descriptor);

		if (bindings.contains(descriptor.GetBinding()))
		{
			/* The member offset has the precalculated alligned offset, so we only have to make sure it's not violated and then just add the size. */
			size = max(size, descriptor.GetInfo().Decorations.MemberOffset);
			size += descriptor.GetSize();
		}
		else
		{
			/* This is a new binding so take the alligned offset into account, and then just add the size. */
			bindings.emplace_back(descriptor.GetBinding());
			size = descriptor.GetAllignedOffset(size) + descriptor.GetSize();
		}
	}

	/* Create a dynamic buffer because the contents of this will most likely change a lot, I should update this to allow for a single stage uniform block. */
	target = new DynamicBuffer(pool.GetSubpass().shaders.front()->GetDevice(), size, BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst);
}

Pu::UniformBlock::UniformBlock(UniformBlock && value)
	: DescriptorSet(std::move(value)), descriptors(std::move(value.descriptors)),
	IsDirty(value.IsDirty), firstUpdate(value.firstUpdate)
{
	target = value.target;
	value.target = nullptr;
}

Pu::UniformBlock & Pu::UniformBlock::operator=(UniformBlock && other)
{
	if (this != &other)
	{
		Destroy();

		DescriptorSet::operator=(std::move(other));
		descriptors = std::move(other.descriptors);
		IsDirty = other.IsDirty;
		firstUpdate = other.firstUpdate;
		target = other.target;

		other.target = nullptr;
	}

	return *this;
}

void Pu::UniformBlock::Update(CommandBuffer & cmdBuffer)
{
	/* Only update the GPU data if needed. */
	if (IsDirty)
	{
		/* Ask the derived uniform block to upload its data to the staging buffer. */
		target->BeginMemoryTransfer();
		Stage(reinterpret_cast<byte*>(target->GetHostMemory()));
		target->EndMemoryTransfer();

		/* Copy the staging buffer into the uniform buffer. */
		target->Update(cmdBuffer);

		/* 
		The descriptor only needs to be set once and the memory barrier is only needed the first time.
		This is because the buffer will remain in uniform read mode throughout the program (after the first update),
		and the descriptor just keeps pointing to the same resource.
		*/
		if (firstUpdate)
		{
			cmdBuffer.MemoryBarrier(*target, PipelineStageFlag::Transfer, PipelineStageFlag::VertexShader, AccessFlag::UniformRead);
			Write(descriptors, *target);
			firstUpdate = false;
		}

		IsDirty = false;
	}
}

void Pu::UniformBlock::Destroy(void)
{
	if (target) delete target;
}