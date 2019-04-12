#include "Graphics/Models/UniformBlock.h"

Pu::UniformBlock::UniformBlock(LogicalDevice & device, size_t size, const DescriptorPool & pool, uint32 set)
	: IsDirty(true), firstUpdate(true)
{
	descriptor = new DescriptorSet(std::move(pool.Allocate(set)));
	target = new DynamicBuffer(device, size, BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst);
}

Pu::UniformBlock::UniformBlock(UniformBlock && value)
	: IsDirty(value.IsDirty), firstUpdate(value.firstUpdate)
{
	descriptor = value.descriptor;
	target = value.target;
	value.target = nullptr;
}

Pu::UniformBlock & Pu::UniformBlock::operator=(UniformBlock && other)
{
	if (this != &other)
	{
		Destroy();
		IsDirty = other.IsDirty;
		firstUpdate = other.firstUpdate;

		descriptor = other.descriptor;
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
			UpdateDescriptor(*descriptor, *target);
			firstUpdate = false;
		}

		IsDirty = false;
	}
}

void Pu::UniformBlock::Destroy(void)
{
	if (descriptor) delete descriptor;
	if (target) delete target;
}