#pragma once
#include "Graphics/Vulkan/Buffer.h"

namespace Pu
{
	/* Defines a subset of a buffers data. */
	class BufferView
	{
	public:
		/* Initializes a new instance of a buffer view spanning the entire buffer. */
		BufferView(_In_ Buffer &buffer, _In_ size_t stride);
		/* Initializes a new instance of a buffer view. */
		BufferView(_In_ Buffer &buffer, _In_ size_t offset, _In_ size_t size, _In_ size_t stride);
		/* Copy constructor. */
		BufferView(_In_ const BufferView &value);
		/* Move constructor. */
		BufferView(_In_ BufferView &&value);

		_Check_return_ BufferView& operator =(_In_ const BufferView&) = delete;
		/* Move assignment. */
		_Check_return_ BufferView& operator =(_In_ BufferView &&other);

		/* Gets the underlying buffer of this view. */
		_Check_return_ inline Buffer& GetBuffer(void)
		{
			return buffer;
		}
		
		/* Gets the underlying buffer of this view. */
		_Check_return_ inline const Buffer& GetBuffer(void) const
		{
			return buffer;
		}

		/* Gets the amount of elements in this buffer view. */
		_Check_return_ inline size_t GetElementCount(void) const
		{
			return size / stride;
		}

	private:
		friend class BufferAccessor;
		friend class CommandBuffer;

		Buffer &buffer;
		size_t offset, size, stride;

		void SetData(const void *data, size_t internalOffset, size_t elementSize, size_t elementCount);
	};
}