#pragma once
#include "Graphics/Vulkan/CommandBuffer.h"

namespace Pu
{
	class StagingBuffer;

	/* Defines a buffer which data will be updated multiple times. */
	class DynamicBuffer
		: public Buffer
	{
	public:
		/* Initializes a new instance of a memory buffer of a specified size (in bytes). */
		DynamicBuffer(_In_ LogicalDevice &device, _In_ size_t size, _In_ BufferUsageFlags usage);
		DynamicBuffer(_In_ const DynamicBuffer&) = delete;
		/* Move constructor. */
		DynamicBuffer(_In_ DynamicBuffer &&value);
		/* Releases the resources allocated by the buffer. */
		~DynamicBuffer(void)
		{
			Destroy();
		}

		_Check_return_ DynamicBuffer& operator =(_In_ const DynamicBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ DynamicBuffer& operator =(_In_ DynamicBuffer &&other);

		/* Updates the dynamic buffer if needed. */
		virtual void Update(_In_ CommandBuffer &cmdBuffer);
		/* Starts the process of transfering data from the CPU to this buffer. */
		virtual void BeginMemoryTransfer(void) override;
		/* Gets the host mapped memory pointer. */
		virtual const void* GetHostMemory(void) const override;
		/* Gets the host mapped memory pointer. */
		virtual void* GetHostMemory(void) override;
		/* Ends the process of transfering data from the CPU to this buffer. */
		virtual void EndMemoryTransfer(void) override;
		/* Ensures that the contents of the CPU buffer as visible to the GPU. */
		virtual void Flush(_In_ DeviceSize size, _In_ DeviceSize offset) override;

	private:
		StagingBuffer *stagingBuffer;
		bool isDirty;

		void Destroy(void);
	};
}