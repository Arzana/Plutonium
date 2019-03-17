#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Buffer::Buffer(LogicalDevice & device, size_t size, BufferUsageFlag usage, bool requiresHostAccess)
	: Asset(true), parent(device), size(size), gpuSize(0), buffer(nullptr), srcAccess(AccessFlag::None), Mutable(true)
{
	memoryProperties = requiresHostAccess ? MemoryPropertyFlag::HostVisible : MemoryPropertyFlag::None;
	Create(BufferCreateInfo(static_cast<DeviceSize>(size), usage));
}

Pu::Buffer::Buffer(Buffer && value)
	: Asset(std::move(value)), parent(value.parent), size(value.size), memoryHndl(value.memoryHndl), bufferHndl(value.bufferHndl), Mutable(value.Mutable),
	memoryProperties(value.memoryProperties), memoryType(value.memoryType), buffer(value.buffer), srcAccess(value.srcAccess)
{
	value.memoryHndl = nullptr;
	value.bufferHndl = nullptr;
	value.buffer = nullptr;
}

Pu::Buffer & Pu::Buffer::operator=(Buffer && other)
{
	if (this != &other)
	{
		Destroy();
		Asset::operator=(std::move(other));

		parent = std::move(other.parent);
		size = other.size;
		memoryHndl = other.memoryHndl;
		bufferHndl = other.bufferHndl;
		memoryProperties = other.memoryProperties;
		memoryType = other.memoryType;
		buffer = other.buffer;
		srcAccess = other.srcAccess;
		Mutable = other.Mutable;

		other.memoryHndl = nullptr;
		other.bufferHndl = nullptr;
		other.buffer = nullptr;
	}

	return *this;
}

void Pu::Buffer::BeginMemoryTransfer(void)
{
	/* Make sure we don't map twice. */
	if (buffer)
	{
		Log::Warning("Attempting to begin memory transfer on already transfering buffer!");
		return;
	}

	/* Make sure the buffer isn't marked as const. */
	if (!Mutable) Log::Fatal("Cannot set data of constant buffer!");

	/* We cannot map data on non-host accessible buffers. */
	if (!IsHostAccessible()) Log::Fatal("Attempting to begin memory transfer on host-invisible buffer!");

	/* Map the entire buffer into memory. */
	Map(size, 0);
}

void Pu::Buffer::EndMemoryTransfer(void)
{
	/* Make sure the buffer was actually started. */
	if (!buffer)
	{
		Log::Error("Attempting to end memory transfer on non-started buffer!");
		return;
	}

	/* Flush the entire buffer and unmap the memory. */
	Flush(WholeSize, 0);
	UnMap();
}

void Pu::Buffer::SetData(const void * data, size_t dataSize, size_t dataStride, size_t offset, size_t stride)
{
	/* Make sure the transfer was started. */
	if (!buffer) Log::Fatal("Attempting to set data on non-started or non-host accessible buffer!");

	/*
	Start at the offset in the destination buffer (i).
	Start at zero in the source buffer (j).
	Loop until either the end of the source or destination buffer is reached.
	Increase the destination buffer by the buffer view stride.
	Increasethe source buffer by the element size (dataStride).
	*/
	for (size_t i = offset, j = 0; i < size && j < dataSize; i += stride, j += dataStride)
	{
		memcpy_s(buffer + i, size - i, reinterpret_cast<const byte*>(data) + j, dataStride);
	}
}

void Pu::Buffer::SetData(const void * data, size_t dataSize, size_t offset)
{
	memcpy_s(buffer + offset, size - offset, data, dataSize);
}

Pu::Asset & Pu::Buffer::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

/* size hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Buffer::Map(size_t size, size_t offset)
{
	VK_VALIDATE(parent.vkMapMemory(parent.hndl, memoryHndl, static_cast<DeviceSize>(offset), static_cast<DeviceSize>(size), 0, reinterpret_cast<void**>(&buffer)), PFN_vkMapMemory);
}
#pragma warning(pop)

void Pu::Buffer::UnMap(void)
{
	/* Resset the buffer back to nullptr to indicate that we no longer have access to the buffer. */
	parent.vkUnmapMemory(parent.hndl, memoryHndl);
	buffer = nullptr;
}

/* size hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Buffer::Flush(size_t size, size_t offset)
{
	/* Flush the section of the buffer indicated by the user. */
	const MappedMemoryRange range{ memoryHndl, static_cast<DeviceSize>(offset), static_cast<DeviceSize>(size) };
	VK_VALIDATE(parent.vkFlushMappedMemoryRanges(parent.hndl, 1, &range), PFN_vkFlushMappedMemoryRanges);
}
#pragma warning(pop)

void Pu::Buffer::Create(const BufferCreateInfo & createInfo)
{
	VK_VALIDATE(parent.vkCreateBuffer(parent.hndl, &createInfo, nullptr, &bufferHndl), PFN_vkCreateBuffer);
	Allocate();
}

void Pu::Buffer::Destroy(void)
{
	Free();
	if (bufferHndl) parent.vkDestroyBuffer(parent.hndl, bufferHndl, nullptr);
}

void Pu::Buffer::Allocate(void)
{
	/* Get the requirements for this block of memory. */
	MemoryRequirements requirements;
	parent.vkGetBufferMemoryRequirements(parent.hndl, bufferHndl, &requirements);

	/* Get the best type of memory available to us. */
	if (parent.parent.GetBestMemoryType(requirements.MemoryTypeBits, memoryProperties, false, memoryType))
	{
		gpuSize = static_cast<size_t>(requirements.Size);

		/* Allocate the memory. */
		const MemoryAllocateInfo info{ requirements.Size, memoryType };
		VK_VALIDATE(parent.vkAllocateMemory(parent.hndl, &info, nullptr, &memoryHndl), PFN_vkAllocateMemory);

		/* Bind the memory to the buffer. */
		Bind();
	}
	else Log::Fatal("Unable to allocate memory for buffer.");
}

void Pu::Buffer::Bind(void)
{
	VK_VALIDATE(parent.vkBindBufferMemory(parent.hndl, bufferHndl, memoryHndl, 0), PFN_vkBindBufferMemory);
}

void Pu::Buffer::Free(void)
{
	if (memoryHndl) parent.vkFreeMemory(parent.hndl, memoryHndl, nullptr);
}