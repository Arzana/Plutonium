#include "Streams/BinaryWriter.h"
#include "Core/Diagnostics/Logging.h"

Pu::BinaryWriter::BinaryWriter(size_t initialCapacity, Endian endian)
	: endian(endian), size(0), capacity(max(initialCapacity, 1ull))
{
	data = reinterpret_cast<byte*>(malloc(capacity));
}

Pu::BinaryWriter::BinaryWriter(const BinaryWriter & value)
	: endian(value.endian), size(value.size), capacity(value.size)
{
	data = reinterpret_cast<byte*>(malloc(capacity));
	memcpy_s(data, capacity, value.data, value.capacity);
}

Pu::BinaryWriter::BinaryWriter(BinaryWriter && value)
	: data(value.data), endian(value.endian), size(value.size), capacity(value.size)
{
	value.capacity = 0;
	value.size = 0;
	value.data = nullptr;
}

Pu::BinaryWriter::~BinaryWriter(void)
{
	if (data) free(data);
}

Pu::BinaryWriter & Pu::BinaryWriter::operator=(const BinaryWriter & other)
{
	if (this != &other)
	{
		if (capacity < other.size) data = reinterpret_cast<byte*>(realloc(data, other.size));
		endian = other.endian;
		size = other.size;
		capacity = max(capacity, other.size);
		memcpy_s(data, capacity, other.data, other.size);
	}

	return *this;
}

Pu::BinaryWriter & Pu::BinaryWriter::operator=(BinaryWriter && other)
{
	if (this != &other)
	{
		if (data) free(data);

		data = other.data;
		endian = other.endian;
		size = other.size;
		capacity = other.capacity;

		other.size = 0;
		other.capacity = 0;
		other.data = nullptr;
	}

	return *this;
}

void Pu::BinaryWriter::Write(bool value)
{
	EnsureCapacity(sizeof(byte));
	data[size++] = static_cast<byte>(value ? 1 : 0);
}

void Pu::BinaryWriter::Write(byte value)
{
	EnsureCapacity(sizeof(byte));
	data[size++] = value;
}

void Pu::BinaryWriter::Write(int8 value)
{
	Write(*reinterpret_cast<byte*>(&value));
}

void Pu::BinaryWriter::Write(int16 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(int16));
}

void Pu::BinaryWriter::Write(uint16 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(uint16));
}

void Pu::BinaryWriter::Write(int32 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(int32));
}

void Pu::BinaryWriter::Write(uint32 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(uint32));
}

void Pu::BinaryWriter::Write(int64 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(int64));
}

void Pu::BinaryWriter::Write(uint64 value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(uint64));
}

void Pu::BinaryWriter::Write(float value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(float));
}

void Pu::BinaryWriter::Write(double value)
{
	Write(reinterpret_cast<byte*>(&value), 0, sizeof(double));
}

void Pu::BinaryWriter::Write(Vector2 value)
{
	EnsureCapacity(sizeof(Vector2));
	Write(value.X);
	Write(value.Y);
}

void Pu::BinaryWriter::Write(Vector3 value)
{
	EnsureCapacity(sizeof(Vector3));
	Write(value.X);
	Write(value.Y);
	Write(value.Z);
}

void Pu::BinaryWriter::Write(Vector4 value)
{
	EnsureCapacity(sizeof(Vector4));
	Write(value.X);
	Write(value.Y);
	Write(value.Z);
	Write(value.W);
}

void Pu::BinaryWriter::Write(Quaternion value)
{
	EnsureCapacity(sizeof(Quaternion));
	Write(value.I);
	Write(value.J);
	Write(value.K);
	Write(value.R);
}

void Pu::BinaryWriter::Write(const Matrix & value)
{
	EnsureCapacity(sizeof(Matrix));
	for (size_t i = 0; i < 16; i++) Write(value.GetComponents()[i]);
}

void Pu::BinaryWriter::Write(const string & value)
{
	EnsureCapacity(sizeof(string_length_t) + value.length());

	Write(static_cast<string_length_t>(value.length()));
	for (char c : value) Write(*reinterpret_cast<byte*>(&c));
}

void Pu::BinaryWriter::Write(const ustring & value)
{
	EnsureCapacity(sizeof(string_length_t) + value.length() * sizeof(char32));

	Write(static_cast<string_length_t>(value.length()));
	for (char32 c : value) Write(*reinterpret_cast<uint32*>(&c));
}

void Pu::BinaryWriter::Write(const BinaryWriter & other)
{
	if (endian == other.endian)
	{
		/* It's faster to directly copy, then to go through the write functions. */
		EnsureCapacity(other.size);
		memcpy(data + size, other.data, other.size);
		size += other.size;
	}
	else Log::Fatal("Cannot combine two binary writers with different endianness!");
}

void Pu::BinaryWriter::Pad(size_t bytes, byte value)
{
	/* Ignore the call if zero bytes are requested. */
	if (bytes == 0) return;

	EnsureCapacity(bytes);
	memset(data + size, value, bytes);
	size += bytes;
}

void Pu::BinaryWriter::Align(size_t alignment)
{
	Pad(size % alignment);
}

void Pu::BinaryWriter::EnsureCapacity(size_t requiredAddition)
{
	if (capacity - size < requiredAddition)
	{
		data = reinterpret_cast<byte*>(realloc(data, capacity += requiredAddition));
	}
}

/* Data hides class member, checked and working as intended. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::BinaryWriter::Write(const byte * data, size_t offset, size_t amount)
{
	EnsureCapacity(amount);

	if (endian == NativeEndian)
	{
		memcpy(this->data + size, data + offset, amount);
		size += amount;
	}
	else
	{
		for (size_t i = offset + amount; i > offset;) this->data[size++] = data[i];
		this->data[size++] = data[offset];
	}
}
#pragma warning (pop)