#pragma once
#include "Graphics/Text/Font.h"
#include "Graphics/Resources/DynamicBuffer.h"
#include "Graphics/Platform/GameWindow.h"

namespace Pu
{
	/* Defines a buffer where the contents can be easily set to a string mesh. */
	class TextBuffer
	{
	public:
		/* Initializes a new instance of a text buffer (the initial size is in characters). */
		TextBuffer(_In_ LogicalDevice &device, _In_ size_t initialSize);
		TextBuffer(_In_ const TextBuffer&) = delete;
		/* Move constructor. */
		TextBuffer(_In_ TextBuffer &&value);
		/* Releases the resources allocated by the buffer. */
		~TextBuffer(void)
		{
			Destroy();
		}

		_Check_return_ TextBuffer& operator =(_In_ const TextBuffer&) = delete;
		/* Move assignment. */
		_Check_return_ TextBuffer& operator =(_In_ TextBuffer &&other);

		/* Updates the text buffer's GPU content if needed. */
		void Update(_In_ CommandBuffer &cmdBuffer);
		/* Updates the text mesh of the text buffer to the specific string for a specific viewport. */
		void SetText(_In_ const ustring &str, _In_ const Font &font, _In_ const GameWindow &wnd);

		/* Gets the amount of vertices in the buffer. */
		_Check_return_ inline uint32 GetCount(void) const
		{
			return count;
		}

	private:
		LogicalDevice *device;
		DynamicBuffer *buffer;
		uint32 count;

		void ReallocBuffer(size_t newSize);
		void AllocBuffer(size_t size);
		void Destroy(void);
	};
}