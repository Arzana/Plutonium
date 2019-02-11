#include "Graphics/Resources/StagingBuffer.h"

Pu::StagingBuffer::StagingBuffer(Buffer & target)
	: Buffer(target.parent, target.GetSize(), BufferUsageFlag::TransferSrc, true)
{}

Pu::StagingBuffer::StagingBuffer(LogicalDevice & device, size_t size)
	: Buffer(device, size, BufferUsageFlag::TransferSrc, true)
{}

Pu::StagingBuffer::StagingBuffer(StagingBuffer && value)
	: Buffer(std::move(value))
{}

Pu::StagingBuffer & Pu::StagingBuffer::operator=(StagingBuffer && other)
{
	if (this != &other)
	{
		Buffer::operator=(std::move(other));
	}

	return *this;
}

void Pu::StagingBuffer::Load(const void * data)
{
	BeginMemoryTransfer();
	SetData(data, size, 0);
	EndMemoryTransfer();
}