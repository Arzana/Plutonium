#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a Vulkan memory buffer. */
	class Buffer
	{
	public:
		/* Initializes a new instance of a buffer with exclusive sharing options. */
		Buffer(_In_ LogicalDevice &device, _In_ size_t size, _In_ BufferUsageFlag usage, _In_opt_ bool requiresHostAccess = false);
		/* Initializes a new instance of a buffer that can be shared by the specified queue families. */
		Buffer(_In_ LogicalDevice &device, _In_ size_t size, _In_ BufferUsageFlag usage, _In_ const vector<uint32> &queueFamilies, _In_opt_ bool requiresHostAccess = false);
		Buffer(_In_ const Buffer&) = delete;
		/* Move constructor. */
		Buffer(_In_ Buffer &&value);
		/* Destroys the buffer. */
		~Buffer(void)
		{
			Destroy();
		}

		_Check_return_ Buffer& operator =(_In_ const Buffer&) = delete;
		/* Move assignment. */
		_Check_return_ Buffer& operator =(_In_ Buffer &&other);

		/* Gets the size (in bytes) of the buffer. */
		_Check_return_ inline DeviceSize GetSize(void) const
		{
			return static_cast<DeviceSize>(size);
		}

		/* Gets the amount of elements in the buffer. */
		_Check_return_ inline uint32 GetElementCount(void) const
		{
			return static_cast<uint32>(elements);
		}

		/* Gets the handle for the Vulkan buffer object. */
		_Check_return_ inline BufferHndl GetHandle(void) const
		{
			return bufferHndl;
		}

		/* Sets the data of the buffer. */
		template <typename _Ty>
		inline void SetData(_In_ const _Ty *data, _In_ size_t count)
		{
			elements = count;
			BufferData(sizeof(_Ty) * count, 0, data);
		}

	private:
		friend class CommandBuffer;

		BufferHndl bufferHndl;
		DeviceMemoryHndl memoryHndl;
		LogicalDevice &parent;
		size_t size, elements;
		bool hostAccess;
		mutable AccessFlag access;

		void BufferData(size_t size, size_t offset, const void *data);
		void Flush(uint32 count, const MappedMemoryRange *ranges);
		void Map(size_t size, size_t offset, void **data);
		void Unmap(void);
		void Create(const BufferCreateInfo &createInfo, MemoryPropertyFlag flags);
		void Bind(void) const;
		MemoryRequirements GetMemoryRequirements(void);
		void Destroy(void);
	};
}