#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Buffer::Buffer(LogicalDevice & device, size_t size, BufferUsageFlags usage, MemoryPropertyFlags requiredProperties, MemoryPropertyFlags optionalProperties)
	: Asset(true), parent(&device), size(size), gpuSize(0), buffer(nullptr),
	srcAccess(AccessFlags::None), Mutable(true), memoryProperties(requiredProperties)
{
	Create(BufferCreateInfo(static_cast<DeviceSize>(size), usage), optionalProperties);
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

		parent = other.parent;
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

Pu::DeviceSize Pu::Buffer::GetLazyMemory(void) const
{
	if (memoryHndl)
	{
		DeviceSize result = 0;
		parent->vkGetDeviceMemoryCommitment(parent->hndl, memoryHndl, &result);
		return result;
	}
	else return 0;
}

void Pu::Buffer::BeginMemoryTransfer(void)
{
#ifdef _DEBUG
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
#endif

	/* Map the entire buffer into memory. */
	Map(size, 0);
}

const void * Pu::Buffer::GetHostMemory(void) const
{
#ifdef _DEBUG
	if (!buffer) Log::Fatal("Attempting to get host memory on buffer that has no mapped memory!");
#endif
	return buffer;
}

void * Pu::Buffer::GetHostMemory(void)
{
#ifdef _DEBUG
	if (!buffer) Log::Fatal("Attempting to get host memory on buffer that has no mapped memory!");
#endif
	return buffer;
}

void Pu::Buffer::EndMemoryTransfer(void)
{
	/* Just flush the entire buffer. */
	Flush(WholeSize, 0);
	UnMap();
}

/* size hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Buffer::Flush(DeviceSize size, DeviceSize offset)
{
#ifdef _DEBUG
	/* Make sure the buffer was actually started. */
	if (!buffer)
	{
		Log::Error("Attempting to flush buffer on non-started buffer!");
		return;
	}
#endif

	/* We don't have to flush if the buffer is Host Coherent. */
	if (!_CrtEnumCheckFlag(memoryProperties, MemoryPropertyFlags::HostCoherent))
	{
		/* Flush the section of the buffer indicated by the user. */
		const MappedMemoryRange range{ memoryHndl, offset, size };
		VK_VALIDATE(parent->vkFlushMappedMemoryRanges(parent->hndl, 1, &range), PFN_vkFlushMappedMemoryRanges);
	}
}
#pragma warning(pop)

void Pu::Buffer::SetData(const void * data, size_t dataSize, size_t dataStride, size_t offset, size_t stride)
{
#ifdef _DEBUG
	/* Make sure the transfer was started. */
	if (!buffer) Log::Fatal("Attempting to set data on non-started or non-host accessible buffer!");
#endif

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
	VK_VALIDATE(parent->vkMapMemory(parent->hndl, memoryHndl, static_cast<DeviceSize>(offset), static_cast<DeviceSize>(size), 0, reinterpret_cast<void**>(&buffer)), PFN_vkMapMemory);
}
#pragma warning(pop)

void Pu::Buffer::UnMap(void)
{
	/* Resset the buffer back to nullptr to indicate that we no longer have access to the buffer. */
	parent->vkUnmapMemory(parent->hndl, memoryHndl);
	buffer = nullptr;
}

void Pu::Buffer::Create(const BufferCreateInfo & createInfo, MemoryPropertyFlags optional)
{
	VK_VALIDATE(parent->vkCreateBuffer(parent->hndl, &createInfo, nullptr, &bufferHndl), PFN_vkCreateBuffer);
	Allocate(optional);
}

void Pu::Buffer::Destroy(void)
{
	Free();
	if (bufferHndl) parent->vkDestroyBuffer(parent->hndl, bufferHndl, nullptr);
}

void Pu::Buffer::Allocate(MemoryPropertyFlags optional)
{
	/* Get the requirements for this block of memory. */
	MemoryRequirements requirements;
	parent->vkGetBufferMemoryRequirements(parent->hndl, bufferHndl, &requirements);

	/* Get the best type of memory available to us. */
	if (parent->parent->GetBestMemoryType(requirements.MemoryTypeBits, memoryProperties, optional, memoryType))
	{
		/* Only log wasted memory if desired, the buffer probably doesn't have a name at this point. */
		gpuSize = static_cast<size_t>(requirements.Size);
		if constexpr (LogWastedMemory)
		{
			if (gpuSize > size)
			{
				if (HasName()) Log::Warning("Buffer '%s'is wasting %zu bytes!", GetName().c_str(), gpuSize - size);
				else Log::Warning("Buffer 0x%X is wasting %zu bytes!", bufferHndl, gpuSize - size);
			}
		}

		/* Allocate the memory. */
		const MemoryAllocateInfo info{ requirements.Size, memoryType };
		++parent->parent->memAllocs;
		VK_VALIDATE(parent->vkAllocateMemory(parent->hndl, &info, nullptr, &memoryHndl), PFN_vkAllocateMemory);

		/* Bind the memory to the buffer. */
		Bind();
	}
	else Log::Fatal("Unable to allocate memory for buffer.");
}

void Pu::Buffer::Bind(void)
{
	VK_VALIDATE(parent->vkBindBufferMemory(parent->hndl, bufferHndl, memoryHndl, 0), PFN_vkBindBufferMemory);
}

void Pu::Buffer::Free(void)
{
	if (memoryHndl)
	{
		parent->vkFreeMemory(parent->hndl, memoryHndl, nullptr);
		--parent->parent->memAllocs;
	}
}