#include "Graphics/Resources/DynamicBuffer.h"
#include "Graphics/Resources/StagingBuffer.h"

Pu::DynamicBuffer::DynamicBuffer(LogicalDevice & device, size_t size, BufferUsageFlag usage)
	: Buffer(device, size, usage, MemoryPropertyFlag::None), isDirty(false)
{
	SetDebugName("DynamicBuffer");
	stagingBuffer = new StagingBuffer(*this);
}

Pu::DynamicBuffer::DynamicBuffer(DynamicBuffer && value)
	: Buffer(std::move(value)), isDirty(value.isDirty), stagingBuffer(value.stagingBuffer)
{
	value.stagingBuffer = nullptr;
}

Pu::DynamicBuffer & Pu::DynamicBuffer::operator=(DynamicBuffer && other)
{
	if (this != &other)
	{
		Destroy();

		Buffer::operator=(std::move(other));
		isDirty = other.isDirty;
		stagingBuffer = other.stagingBuffer;

		other.stagingBuffer = nullptr;
	}

	return *this;
}

void Pu::DynamicBuffer::Update(CommandBuffer & cmdBuffer)
{
	/* Only update the GPU data if needed. */
	if (isDirty)
	{
		cmdBuffer.CopyEntireBuffer(*stagingBuffer, *this);
		isDirty = false;
	}
}

void Pu::DynamicBuffer::BeginMemoryTransfer(void)
{
	stagingBuffer->BeginMemoryTransfer();
	isDirty = true;
}

const void * Pu::DynamicBuffer::GetHostMemory(void) const
{
	return stagingBuffer->GetHostMemory();
}

void * Pu::DynamicBuffer::GetHostMemory(void)
{
	return stagingBuffer->GetHostMemory();
}

void Pu::DynamicBuffer::EndMemoryTransfer(void)
{
	stagingBuffer->EndMemoryTransfer();
}

void Pu::DynamicBuffer::Destroy(void)
{
	if (stagingBuffer) delete stagingBuffer;
}