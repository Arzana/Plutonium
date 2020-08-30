#include "Graphics/Resources/StagingBuffer.h"

Pu::StagingBuffer::StagingBuffer(Buffer & target)
	: Buffer(*target.parent, target.GetSize(), BufferUsageFlags::TransferSrc, MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent)
{
	SetDebugName("StagingBuffer");
}

Pu::StagingBuffer::StagingBuffer(LogicalDevice & device, size_t size)
	: Buffer(device, size, BufferUsageFlags::TransferSrc, MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent)
{
	SetDebugName("StagingBuffer");
}

void Pu::StagingBuffer::EndMemoryTransfer(void)
{
	Buffer::EndMemoryTransfer();

	/*
	Mark the staging buffer as loaded, making it ready for use.
	This should never be called via a loader so don't mark it as such.
	*/
	MarkAsLoaded(false, L"StagingBuffer");
}

void Pu::StagingBuffer::Load(const void * data)
{
	/* Transfer the data to the CPU-GPU accessible underlying buffer. */
	BeginMemoryTransfer();
	SetData(data, size, 0);
	EndMemoryTransfer();
}