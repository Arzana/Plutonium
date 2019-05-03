#pragma once
#include "StreamReader.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines an object that can read basic types from a binary source buffer. */
	class BinaryReader
		: public StreamReader
	{
	public:
		/* Initializes a new instance of a binary reader. */
		BinaryReader(_In_ const void *source, _In_ size_t size, _In_opt_ Endian endian = NativeEndian);
		/* Copy constructor. */
		BinaryReader(_In_ const BinaryReader &value);
		/* Move constructor. */
		BinaryReader(_In_ BinaryReader &&value);

		/* Copy assignment. */
		_Check_return_ BinaryReader& operator =(const BinaryReader &other);
		/* Move assignment. */
		_Check_return_ BinaryReader& operator =(BinaryReader &&other);

		/* Reads the next byte from the buffer as a boolean and increases the position by one byte. */
		_Check_return_ bool ReadBool(void);
		/* Reads the next byte from the buffer and increases the position by one byte. */
		_Check_return_ byte ReadByte(void);
		/* Reads the next byte from the buffer as an signed byte and increases the position by one byte. */
		_Check_return_ int8 ReadSByte(void);
		/* Reads the next two bytes from the buffer as an short integer and increases the position by two bytes. */
		_Check_return_ int16 ReadInt16(void);
		/* Reads the next two bytes from the buffer as an unsigned short integer and increases the position by two bytes. */
		_Check_return_ uint16 ReadUInt16(void);
		/* Reads the next four bytes from the buffer as an integer and increases the position by four bytes. */
		_Check_return_ int32 ReadInt32(void);
		/* Reads the next four bytes from the buffer as an unsigned integer and increases the position by four bytes. */
		_Check_return_ uint32 ReadUInt32(void);
		/* Reads the next eight bytes from the buffer as an long integer and increases the position by eight bytes. */
		_Check_return_ int64 ReadInt64(void);
		/* Reads the next eight bytes from the buffer as an unsigned long integer and increases the position by eight bytes. */
		_Check_return_ uint64 ReadUInt64(void);
		/* Reads the next four bytes from the buffer as a single precision floating point and increases the position by four bytes. */
		_Check_return_ float ReadSingle(void);
		/* Reads the next eight bytes from the buffer as a double precision floating point and increases the position by eight bytes. */
		_Check_return_ double ReadDouble(void);
		/* Reads the next eight bytes from the buffer as a 2D floating point vector and increases the position by eight bytes. */
		_Check_return_ Vector2 ReadVector2(void);
		/* Reads the next 12 bytes from the buffer as a 3D floating point vector and increases the position by 12 bytes. */
		_Check_return_ Vector3 ReadVector3(void);
		/* Reads the next 16 bytes from the buffer as a 4D floating point vector and increases the position by 16 bytes. */
		_Check_return_ Vector4 ReadVector4(void);
		/* Reads the next 16 bytes form the buffer as a quaternion and increases the position by 16 bytes. */
		_Check_return_ Quaternion ReadQuaternion(void);
		/* Reads the next 64 bytes from the buffer as a matrix and increases the position by 64 bytes. */
		_Check_return_ Matrix ReadMatrix(void);
		/* Reads a variable amount of bytes from the buffer as a length first ANSI string and increases the position. */
		_Check_return_ string ReadString(void);
		/* Reads a variable amount of bytes from the buffer as a length first UTF-32 string and increases the position. */
		_Check_return_ ustring ReadUString(void);
		/*
		Reads the next byte from the stream.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Read(void);
		/*
		Reads a specified amount of bytes from the stream.
		Returns the amount of bytes read.
		*/
		_Check_return_ virtual size_t Read(_Out_ byte *buffer, _In_ size_t offset, _In_ size_t amount);

		/* Reads the next byte from the buffer as a boolean. */
		_Check_return_ bool PeekBool(void);
		/* Reads the next byte from the buffer. */
		_Check_return_ byte PeekByte(void);
		/* Reads the next byte from the buffer as an signed byte. */
		_Check_return_ int8 PeekSByte(void);
		/* Reads the next two bytes from the buffer as an short integer. */
		_Check_return_ int16 PeekInt16(void);
		/* Reads the next two bytes from the buffer as an unsigned short integer. */
		_Check_return_ uint16 PeekUInt16(void);
		/* Reads the next four bytes from the buffer as an integer. */
		_Check_return_ int32 PeekInt32(void);
		/* Reads the next four bytes from the buffer as an unsigned integer. */
		_Check_return_ uint32 PeekUInt32(void);
		/* Reads the next eight bytes from the buffer as an long integer. */
		_Check_return_ int64 PeekInt64(void);
		/* Reads the next eight bytes from the buffer as an unsigned long integer. */
		_Check_return_ uint64 PeekUInt64(void);
		/* Reads the next four bytes from the buffer as a single precision floating point. */
		_Check_return_ float PeekSingle(void);
		/* Reads the next eight bytes from the buffer as a double precision floating point. */
		_Check_return_ double PeekDouble(void);
		/* Reads the next eight bytes from the buffer as a 2D floating point vector. */
		_Check_return_ Vector2 PeekVector2(void);
		/* Reads the next 12 bytes from the buffer as a 3D floating point vector. */
		_Check_return_ Vector3 PeekVector3(void);
		/* Reads the next 16 bytes from the buffer as a 4D floating point vector. */
		_Check_return_ Vector4 PeekVector4(void);
		/* Reads the next 16 bytes form the buffer as a quaternion. */
		_Check_return_ Quaternion PeekQuaternion(void);
		/* Reads the next 64 bytes from the buffer as a matrix. */
		_Check_return_ Matrix PeekMatrix(void);
		/* Reads a variable amount of bytes from the buffer as a length first ANSI string. */
		_Check_return_ string PeekString(void);
		/* Reads a variable amount of bytes from the buffer as a length first UTF-32 string. */
		_Check_return_ ustring PeekUString(void);
		/*
		Reads the next byte from the stream without increasing the read position.
		Returns -1 when no more bytes could be read.
		*/
		_Check_return_ virtual int32 Peek(void);
		/*
		Reads a specified amount of bytes from the stream without increasing the read position.
		Returns the actual amount of bytes peeked.
		*/
		_Check_return_ virtual size_t Peek(_Out_ byte *buffer, _In_ size_t offset, _In_ size_t amount);

		/*
		Seeks the stream, increasing it's read position by a sepcified amount.
		*/
		virtual void Seek(_In_ SeekOrigin from, _In_ int64 amount);
		/* Advances the reading position as if reading a the specified type. */
		template <typename _Ty>
		inline void Advance(void)
		{
			Seek(SeekOrigin::Current, static_cast<int64>(sizeof(_Ty)));
		}

		/* Gets the size (in bytes) of the binary stream. */
		_Check_return_ inline size_t GetSize(void) const
		{
			return size;
		}

		/* Gets the current read position (in bytes) of the reader. */
		_Check_return_ inline size_t GetLocation(void) const
		{
			return position;
		}

		/* Gets the underlying data stream. */
		_Check_return_ inline const void *GetData(void) const
		{
			return data;
		}

		/* Gets the underlying data stream starting from the current position. */
		_Check_return_ inline const void *GetDataLocation(void) const
		{
			return data + position;
		}

		/* Changes the endianess of the binary reader. */
		inline void ResetEndian(_In_ Endian newEndian)
		{
			endian = newEndian;
		}

	private:
		const byte *data;
		size_t size, position;
		Endian endian;

		bool CheckOverflow(size_t bytesNeeded, bool raise);
	};

	/* Quickly converts the raw bytes to a Vector2. */
	_Check_return_ inline Vector2 b2vec2(_In_ const void *data)
	{
		return BinaryReader(data, sizeof(Vector2)).ReadVector2();
	}

	/* Quickly converts the raw bytes to a Vector3. */
	_Check_return_ inline Vector3 b2vec3(_In_ const void *data)
	{
		return BinaryReader(data, sizeof(Vector3)).ReadVector3();
	}

	/* Quickly converts the raw bytes to a Vector4. */
	_Check_return_ inline Vector4 b2vec4(_In_ const void *data)
	{
		return BinaryReader(data, sizeof(Vector4)).ReadVector4();
	}

	/* Quickly converts the raw bytes to a Quaternion. */
	_Check_return_ inline Quaternion b2quat(_In_ const void *data)
	{
		return BinaryReader(data, sizeof(Quaternion)).ReadQuaternion();
	}

	/* Quickly converts the raw bytes to a Matrix. */
	_Check_return_ inline Matrix b2mtrx(_In_ const void *data)
	{
		return BinaryReader(data, sizeof(Matrix)).ReadMatrix();
	}
}