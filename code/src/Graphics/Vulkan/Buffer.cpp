#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Buffer::Buffer(LogicalDevice & device, size_t size, BufferUsageFlag usage, bool requiresHostAccess)
	: parent(device), size(size), hostAccess(requiresHostAccess), elements(0)
{
	const BufferCreateInfo createInfo(static_cast<DeviceSize>(size), usage);
	Create(createInfo, requiresHostAccess ? MemoryPropertyFlag::HostVisible : MemoryPropertyFlag::None);
}

Pu::Buffer::Buffer(LogicalDevice & device, size_t size, BufferUsageFlag usage, const vector<uint32>& queueFamilies, bool requiresHostAccess)
	: parent(device), size(size), hostAccess(requiresHostAccess), elements(0)
{
	const BufferCreateInfo createInfo(static_cast<DeviceSize>(size), usage, queueFamilies);
	Create(createInfo, requiresHostAccess ? MemoryPropertyFlag::HostVisible : MemoryPropertyFlag::None);
}

Pu::Buffer::Buffer(Buffer && value)
	: parent(value.parent), bufferHndl(value.bufferHndl), memoryHndl(value.memoryHndl), size(value.size), elements(value.elements), hostAccess(value.hostAccess)
{
	value.bufferHndl = nullptr;
	value.memoryHndl = nullptr;
}

Pu::Buffer & Pu::Buffer::operator=(Buffer && other)
{
	if (this != &other)
	{
		Destroy();

		parent = std::move(other.parent);
		bufferHndl = other.bufferHndl;
		memoryHndl = other.memoryHndl;
		size = other.size;
		elements = other.elements;
		hostAccess = other.hostAccess;

		other.bufferHndl = nullptr;
		other.memoryHndl = nullptr;
		other.size = 0;
		other.elements = 0;
	}

	return *this;
}

/* size hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Buffer::BufferData(size_t size, size_t offset, const void * data)
{
	/* Requires the shared memory buffer. */
	void *buffer;
	Map(size, offset, &buffer);

	/* Copy the data into the Vulkan shared buffer. */
	memcpy_s(buffer, size, data, size);

	/* Indicate to Vulkan that we changed the whole buffer size. */
	const MappedMemoryRange range(memoryHndl);
	Flush(1, &range);

	/* Indicate to Vulkan that we no longer need CPU access to the buffer. */
	Unmap();
}
#pragma warning(pop)

void Pu::Buffer::Flush(uint32 count, const MappedMemoryRange * ranges)
{
	VK_VALIDATE(parent.vkFlushMappedMemoryRanges(parent.hndl, count, ranges), PFN_vkFlushMappedMemoryRanges);
}

/* size hides class member, checked and works as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Buffer::Map(size_t size, size_t offset, void ** data)
{
	if (!hostAccess) Log::Fatal("Cannot map memory to a buffer that doesn't have host access enabled!");
	VK_VALIDATE(parent.vkMapMemory(parent.hndl, memoryHndl, static_cast<DeviceSize>(offset), static_cast<DeviceSize>(size), 0, data), PFN_vkMapMemory);
}
#pragma warning(pop)

void Pu::Buffer::Unmap(void)
{
	parent.vkUnmapMemory(parent.hndl, memoryHndl);
}

void Pu::Buffer::Create(const BufferCreateInfo & createInfo, MemoryPropertyFlag flags)
{
	/* Create the buffer object. */
	VK_VALIDATE(parent.vkCreateBuffer(parent.hndl, &createInfo, nullptr, &bufferHndl), PFN_vkCreateBuffer);

	/* Find the type of memory that best supports our needs. */
	uint32 typeIdx;
	const MemoryRequirements requirements = GetMemoryRequirements();
	if (parent.parent.GetBestMemoryType(requirements.MemoryTypeBits, flags, typeIdx))
	{
		/* Allocate the buffer's data. */
		const MemoryAllocateInfo allocateInfo(requirements.Size, typeIdx);
		VK_VALIDATE(parent.vkAllocateMemory(parent.hndl, &allocateInfo, nullptr, &memoryHndl), PFN_vkAllocateMemory);

		/* Bind the memory to the buffer. */
		Bind();
	}
	else Log::Fatal("Unable to allocate memory for buffer!");
}

void Pu::Buffer::Bind(void) const
{
	// TODO: We should not allocate a memory object for ever buffer but rather make one big one and set a proper offset to increase cache hits.
	VK_VALIDATE(parent.vkBindBufferMemory(parent.hndl, bufferHndl, memoryHndl, 0), PFN_vkBindBufferMemory);
}

Pu::MemoryRequirements Pu::Buffer::GetMemoryRequirements(void)
{
	MemoryRequirements result;
	parent.vkGetBufferMemoryRequirements(parent.hndl, bufferHndl, &result);
	return result;
}

void Pu::Buffer::Destroy(void)
{
	if (bufferHndl) parent.vkDestroyBuffer(parent.hndl, bufferHndl, nullptr);
	if (memoryHndl) parent.vkFreeMemory(parent.hndl, memoryHndl, nullptr);
}