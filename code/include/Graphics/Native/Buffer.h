#pragma once
#include "BindTargets.h"
#include "BufferUsage.h"
#include "Core\SafeMemory.h"
#include "Graphics\Rendering\Attribute.h"
#include "Graphics\Native\Window.h"

namespace Plutonium
{
	/* Defines a GPU side buffer. */
	struct Buffer
	{
	public:
		/* Initializes a new instance of a GPU buffer. */
		Buffer(_In_ WindowHandler wnd, _In_ BindTarget target);
		Buffer(_In_ const Buffer &value) = delete;
		Buffer(_In_ Buffer &&value) = delete;
		/* Releases the reources allocated by the buffer. */
		~Buffer(void) noexcept;

		_Check_return_ Buffer& operator =(_In_ const Buffer &other) = delete;
		_Check_return_ Buffer& operator =(_In_ Buffer &&other) = delete;

		/* Binds the buffer to be used. */
		void Bind(void) const;
		/* Sets the data for the specified buffer. */
		template <typename _Ty>
		inline void SetData(_In_ BufferUsage usage, _In_ const _Ty *data, _In_ size_t count)
		{
			size = count;
			BufferData(usage, sizeof(_Ty) * count, void_ptr(data));
		}
		/* Updates the data for a specified buffer. */
		template <typename _Ty>
		inline void SetData(_In_ const _Ty *data, _In_ size_t count)
		{
			size = count;
			BufferSubData(sizeof(_Ty) * count, void_ptr(data), true);
		}
		/* Updates the data for a specified buffer. */
		template <typename _Ty>
		inline void SetData(_In_ const _Ty *data)
		{
			BufferSubData(sizeof(_Ty) * size, void_ptr(data), false);
		}

		/* Gets the amount of elements stored by this buffer. */
		_Check_return_ inline size_t GetElementCount(void) const
		{
			return size;
		}
		/* Gets the handler associated with the buffer. */
		_Check_return_ inline uint32 GetHandler(void) const
		{
			return hndlr;
		}

	private:
		uint32 hndlr;
		size_t size;
		int64 bsize;
		GLenum type;
		WindowHandler wnd;

		void BufferData(BufferUsage usage, size_t size, const void *data);
		void BufferSubData(size_t size, const void *data, bool sizeUpdated);
	};
}