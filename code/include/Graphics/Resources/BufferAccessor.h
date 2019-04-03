#pragma once
#include "BufferView.h"
#include "Content/GLTFLoader.h"

namespace Pu
{
	/* Defines a way to access individual buffer fields. */
	class BufferAccessor
	{
	public:
		/* Initializes a new instance of a buffer accessor from a predefined type at a predefined internal offset. */
		BufferAccessor(_In_ BufferView &view, _In_ const FieldType &type, _In_ size_t offset);
		/* Copy constructor. */
		BufferAccessor(_In_ const BufferAccessor &value);
		/* Move constructor. */
		BufferAccessor(_In_ BufferAccessor &&value);

		_Check_return_ BufferAccessor& operator =(_In_ const BufferAccessor&) = delete;
		/* Move assignment. */
		_Check_return_ BufferAccessor& operator =(_In_ BufferAccessor &&other);

		/* Gets the underlying buffer view. */
		_Check_return_ inline BufferView& GetView(void)
		{
			return view;
		}

		/* Gets the underlying buffer view. */
		_Check_return_ inline const BufferView& GetView(void) const
		{
			return view;
		}

		/* Gets the amount of element last stored via the accessor. */
		_Check_return_ inline size_t GetElementCount(void) const
		{
			return view.GetElementCount();
		}

		/* Gets the type of the underlying elements. */
		_Check_return_ inline const FieldType& GetElementType(void) const
		{
			return elementType;
		}

		/* Set the data of this accessor. */
		void SetData(_In_ const void *data, _In_ size_t count);

	private:
		friend class Mesh;
		friend class CommandBuffer;

		BufferView &view;
		FieldType elementType;
		size_t offset;
	};
}