#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a Vulkan memory buffer. */
	class Buffer
	{
	public:
		/* Initializes a new instance of a memory buffer of a specified size (in bytes). */
		Buffer(_In_ LogicalDevice &device, _In_ size_t size, _In_ BufferUsageFlag usage, _In_ bool requiresHostAccess);
		Buffer(_In_ const Buffer&) = delete;
		/* Move constructor. */
		Buffer(_In_ Buffer &&value);
		/* Frees the buffer. */
		virtual ~Buffer(void)
		{
			Destroy();
		}

		_Check_return_ Buffer& operator =(_In_ const Buffer&) = delete;
		/* Move assignment. */
		_Check_return_ Buffer& operator =(_In_ Buffer &&other);

		/* Gets whether this buffer's data can be set by the CPU. */
		_Check_return_ inline bool IsHostAccessible(void) const
		{
			return _CrtEnumCheckFlag(memoryProperties, MemoryPropertyFlag::HostVisible);
		}

		/* Gets whether this buffer's data is cached on the host. */
		_Check_return_ inline bool IsCached(void) const
		{
			return _CrtEnumCheckFlag(memoryProperties, MemoryPropertyFlag::HostCached);
		}

		/* Gets the size (in bytes) of this buffer. */
		_Check_return_ inline size_t GetSize(void) const
		{
			return size;
		}

		/* Starts the process of transfering data from the CPU to this buffer. */
		void BeginMemoryTransfer(void);
		/* Ends the process of transfering data from the CPU to this buffer. */
		void EndMemoryTransfer(void);

	protected:
		/* Sets the raw memory of the buffer. */
		void SetData(_In_ const void *data, _In_ size_t dataSize, _In_ size_t dataStride, _In_ size_t offset, _In_ size_t stride);
		/* Sets the raw memory of the buffer. */
		void SetData(_In_ const void *data, _In_ size_t dataSize, _In_ size_t offset);

	private:
		friend class BufferView;
		friend class StagingBuffer;
		friend class CommandBuffer;
		friend class DescriptorSet;

		LogicalDevice &parent;
		mutable AccessFlag srcAccess;

		size_t size, gpuSize;
		DeviceMemoryHndl memoryHndl;
		BufferHndl bufferHndl;
		MemoryPropertyFlag memoryProperties;
		uint32 memoryType;
		void *buffer;

		void Map(size_t size, size_t offset);
		void UnMap(void);
		void Flush(size_t size, size_t offset);

		void Create(const BufferCreateInfo &createInfo);
		void Destroy(void);

		void Allocate(void);
		void Bind(void);
		void Free(void);
	};
}