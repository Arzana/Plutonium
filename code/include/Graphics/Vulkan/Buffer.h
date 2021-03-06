#pragma once
#include "LogicalDevice.h"
#include "Content/Asset.h"

namespace Pu
{
	/* Defines a Vulkan memory buffer. */
	class Buffer
		: public Asset
	{
	public:
		/* Initializes a new instance of a memory buffer of a specified size (in bytes). */
		Buffer(_In_ LogicalDevice &device, _In_ size_t size, _In_ BufferUsageFlags usage, _In_ MemoryPropertyFlags requiredProperties, _In_opt_ MemoryPropertyFlags optionalProperties = MemoryPropertyFlags::None);
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
			return _CrtEnumCheckFlag(memoryProperties, MemoryPropertyFlags::HostVisible);
		}

		/* Gets whether this buffer's data is cached on the host. */
		_Check_return_ inline bool IsCached(void) const
		{
			return _CrtEnumCheckFlag(memoryProperties, MemoryPropertyFlags::HostCached);
		}

		/* Gets whether this buffer allows it's data to be altered. */
		_Check_return_ inline bool CanSetData(void) const
		{
			return Mutable;
		}

		/* Gets the size (in bytes) of this buffer. */
		_Check_return_ inline size_t GetSize(void) const
		{
			return size;
		}

		/* Sets a debuggable name for the buffer. */
		inline void SetDebugName(_In_ const string &name) const
		{
			parent->SetDebugName(ObjectType::Buffer, bufferHndl, name);
		}

		/* Gets the amount of lazily-allocated bytes that are committed for the buffer. */
		_Check_return_ DeviceSize GetLazyMemory(void) const;
		/* Starts the process of transfering data from the CPU to this buffer. */
		virtual void BeginMemoryTransfer(void);
		/* Gets the host mapped memory pointer. */
		virtual const void* GetHostMemory(void) const;
		/* Gets the host mapped memory pointer. */
		virtual void* GetHostMemory(void);
		/* Ends the process of transfering data from the CPU to this buffer. */
		virtual void EndMemoryTransfer(void);
		/* Ensures that the contents of the CPU buffer as visible to the GPU. */
		virtual void Flush(_In_ DeviceSize size, _In_ DeviceSize offset);

	protected:
		/* Whether to allow the user to change the data of this buffer. */
		bool Mutable;

		/* Sets the raw memory of the buffer. */
		void SetData(_In_ const void *data, _In_ size_t dataSize, _In_ size_t dataStride, _In_ size_t offset, _In_ size_t stride);
		/* Sets the raw memory of the buffer. */
		void SetData(_In_ const void *data, _In_ size_t dataSize, _In_ size_t offset);

		/* Duplicates the asset, either returning a reference of itself. */
		_Check_return_ virtual Asset& Duplicate(_In_ AssetCache&);

	private:
		friend class BufferView;
		friend class StagingBuffer;
		friend class CommandBuffer;
		friend class DescriptorSetBase;

		LogicalDevice *parent;
		mutable AccessFlags srcAccess;

		size_t size, gpuSize;
		DeviceMemoryHndl memoryHndl;
		BufferHndl bufferHndl;
		MemoryPropertyFlags memoryProperties;
		uint32 memoryType;
		byte *buffer;

		void Map(size_t size, size_t offset);
		void UnMap(void);

		void Create(const BufferCreateInfo &createInfo, MemoryPropertyFlags optional);
		void Destroy(void);

		void Allocate(MemoryPropertyFlags optional);
		void Bind(void);
		void Free(void);
	};
}