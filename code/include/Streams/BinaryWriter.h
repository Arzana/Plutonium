#pragma once
#include "StreamWriter.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines an object that can write basic types to a binary destination buffer. */
	class BinaryWriter
		: public StreamWriter
	{
	public:
		/* Initializes a new instance of a binary writer. */
		BinaryWriter(_In_ size_t initialCapacity = 0, _In_opt_ Endian endian = NativeEndian);
		/* Copy constructor. */
		BinaryWriter(_In_ const BinaryWriter &value);
		/* Move constructor. */
		BinaryWriter(_In_ BinaryWriter &&value);
		/* Releases the memory allocated by the binary writer. */
		virtual ~BinaryWriter(void);

		/* Copy assignment. */
		_Check_return_ BinaryWriter& operator =(_In_ const BinaryWriter &other);
		/* Move assignment. */
		_Check_return_ BinaryWriter& operator =(_In_ BinaryWriter &&other);

		/* Writes the specified boolean value to the buffer. */
		void Write(_In_ bool value);
		/* Writes the specified unsigned byte value to the buffer. */
		virtual void Write(_In_ byte value);
		/* Writes the specified signed byte value to the buffer. */
		void Write(_In_ int8 value);
		/* Writes the specified signed short integer to the buffer. */
		void Write(_In_ int16 value);
		/* Writes the specified unsigned short integer to the buffer. */
		void Write(_In_ uint16 value);
		/* Writes the specified signed integer to the buffer. */
		void Write(_In_ int32 value);
		/* Writes the specified unsigned integer to the buffer. */
		void Write(_In_ uint32 value);
		/* Writes the specified signed long integer to the buffer. */
		void Write(_In_ int64 value);
		/* Writes the specified unsigned long integer to the buffer. */
		void Write(_In_ uint64 value);
		/* Writes the specified single precision floating point to the buffer. */
		void Write(_In_ float value);
		/* Writes the specified double precistion floating point to the buffer. */
		void Write(_In_ double value);
		/* Writes the specified 2D float point vector to the buffer. */
		void Write(_In_ Vector2 value);
		/* Writes the specified 3D float point vector to the buffer. */
		void Write(_In_ Vector3 value);
		/* Writes the specified 4D float point vector to the buffer. */
		void Write(_In_ Vector4 value);
		/* Writes the specified quaternion to the buffer. */
		void Write(_In_ Quaternion value);
		/* Writes the specified matrix to the buffer. */
		void Write(_In_ const Matrix &value);
		/* Writes the specified ANSI string to the buffer as a length first string. */
		void Write(_In_ const string &value);
		/* Writes the specified UTF-32 string to the buffer as a length first string. */
		void Write(_In_ const ustring &value);
		/* Writes the specified binary data to the stream. */
		virtual void Write(_In_ const byte *data, _In_ size_t offset, _In_ size_t amount);
		/* Ensures that the buffer has enough space for the specified size. */
		void EnsureCapacity(_In_ size_t requiredAddition);

		/* Gets the amount of bytes writen to the buffer. */
		_Check_return_ inline size_t GetSize(void) const
		{
			return size;
		}

		/* Gets the current size (in bytes) of the buffer. */
		_Check_return_ inline size_t GetCapacity(void) const
		{
			return capacity;
		}

		/* Gets the underlying data stream. */
		_Check_return_ inline const byte* GetData(void) const
		{
			return data;
		}

		/* Changes the dnianess of the binary writer. */
		inline void ResetEndian(_In_ Endian newEndian)
		{
			endian = newEndian;
		}

		/* Resets the write position of the buffer to the start. */
		void Reset(void)
		{
			size = 0;
		}

	private:
		byte *data;
		size_t size, capacity;
		Endian endian;
	};
}