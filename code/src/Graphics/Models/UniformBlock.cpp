#include "Graphics/Models/UniformBlock.h"

Pu::UniformBlock::UniformBlock(LogicalDevice & device, size_t size, const DescriptorPool & pool, uint32 set)
	: IsDirty(true), firstUpdate(true)
{
	descriptor = new DescriptorSet(std::move(pool.Allocate(set)));
	targetBuffer = new Buffer(device, size, BufferUsageFlag::UniformBuffer | BufferUsageFlag::TransferDst, false);
	stagingBuffer = new StagingBuffer(*targetBuffer);
}

Pu::UniformBlock::UniformBlock(UniformBlock && value)
	: IsDirty(value.IsDirty), firstUpdate(value.firstUpdate)
{
	descriptor = value.descriptor;
	targetBuffer = value.targetBuffer;
	stagingBuffer = value.stagingBuffer;

	value.stagingBuffer = nullptr;
	value.targetBuffer = nullptr;
	value.stagingBuffer = nullptr;
}

Pu::UniformBlock & Pu::UniformBlock::operator=(UniformBlock && other)
{
	if (this != &other)
	{
		Destroy();
		IsDirty = other.IsDirty;
		firstUpdate = other.firstUpdate;

		descriptor = other.descriptor;
		targetBuffer = other.targetBuffer;
		stagingBuffer = other.stagingBuffer;

		other.stagingBuffer = nullptr;
		other.targetBuffer = nullptr;
		other.stagingBuffer = nullptr;
	}

	return *this;
}

void Pu::UniformBlock::Update(CommandBuffer & cmdBuffer)
{
	/* Only update the GPU data if needed. */
	if (IsDirty)
	{
		/* Ask the derived uniform block to upload its data to the staging buffer. */
		Stage(*stagingBuffer);

		/* Copy the staging buffer into the uniform buffer. */
		cmdBuffer.CopyEntireBuffer(*stagingBuffer, *targetBuffer);

		/* 
		The descriptor only needs to be set once and the memory barrier is only needed the first time.
		This is because the buffer will remain in uniform read mode throughout the program (after the first update),
		and the descriptor just keeps pointing to the same resource.
		*/
		if (firstUpdate)
		{
			cmdBuffer.MemoryBarrier(*targetBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexShader, AccessFlag::UniformRead);
			UpdateDescriptor(*descriptor, *targetBuffer);
			firstUpdate = false;
		}

		IsDirty = false;
	}
}

void Pu::UniformBlock::Destroy(void)
{
	if (descriptor) delete descriptor;
	if (targetBuffer) delete targetBuffer;
	if (stagingBuffer) delete stagingBuffer;
}