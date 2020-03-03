#include "Streams/BinaryReader.h"
#include "Core/Diagnostics/Logging.h"
#include <cstring>

using namespace Pu;

Pu::BinaryReader::BinaryReader(const void * source, size_t size, Endian endian)
	: data(reinterpret_cast<const byte*>(source)), size(size), position(0), endian(endian)
{}

Pu::BinaryReader::BinaryReader(const BinaryReader & value)
	: data(value.data), size(value.size), position(value.position), endian(value.endian)
{}

Pu::BinaryReader::BinaryReader(BinaryReader && value)
	: data(value.data), size(value.size), position(value.position), endian(value.endian)
{
	value.data = nullptr;
	value.size = 0;
}

BinaryReader & Pu::BinaryReader::operator=(const BinaryReader & other)
{
	if (this != &other)
	{
		data = other.data;
		size = other.size;
		position = other.position;
		endian = other.endian;
	}

	return *this;
}

BinaryReader & Pu::BinaryReader::operator=(BinaryReader && other)
{
	if (this != &other)
	{
		data = other.data;
		size = other.size;
		position = other.position;
		endian = other.endian;

		other.data = nullptr;
		other.size = 0;
	}

	return *this;
}

int32 Pu::BinaryReader::Read(void)
{
	return CheckOverflow(1, false) ? static_cast<int32>(ReadByte()) : -1;
}

size_t Pu::BinaryReader::Read(byte * buffer, size_t offset, size_t amount)
{
	const size_t result = Peek(buffer, offset, amount);
	position += result;
	return result;
}

bool Pu::BinaryReader::PeekBool(void) const
{
	CheckOverflow(1, true);
	return data[position] != 0;
}

byte Pu::BinaryReader::PeekByte(void) const
{
	CheckOverflow(1, true);
	return data[position];
}

int8 Pu::BinaryReader::PeekSByte(void) const
{
	CheckOverflow(sizeof(int8), true);
	return *reinterpret_cast<const int8*>(data + position);
}

int16 Pu::BinaryReader::PeekInt16(void) const
{
	CheckOverflow(sizeof(int16), true);
	const int16 raw = *reinterpret_cast<const int16*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

uint16 Pu::BinaryReader::PeekUInt16(void) const
{
	CheckOverflow(sizeof(uint16), true);
	const uint16 raw = *reinterpret_cast<const uint16*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

int32 Pu::BinaryReader::PeekInt32(void) const
{
	CheckOverflow(sizeof(int32), true);
	const int32 raw = *reinterpret_cast<const int32*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

uint32 Pu::BinaryReader::PeekUInt32(void) const
{
	CheckOverflow(4, true);
	const uint32 raw = *reinterpret_cast<const uint32*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

int64 Pu::BinaryReader::PeekInt64(void) const
{
	CheckOverflow(sizeof(int64), true);
	const int64 raw = *reinterpret_cast<const int64*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

uint64 Pu::BinaryReader::PeekUInt64(void) const
{
	CheckOverflow(sizeof(uint64), true);
	const uint64 raw = *reinterpret_cast<const uint64*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

float Pu::BinaryReader::PeekSingle(void) const
{
	CheckOverflow(sizeof(float), true);
	const float raw = *reinterpret_cast<const float*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

double Pu::BinaryReader::PeekDouble(void) const
{
	CheckOverflow(sizeof(double), true);
	const double raw = *reinterpret_cast<const double*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw);
}

Vector2 Pu::BinaryReader::PeekVector2(void) const
{
	CheckOverflow(sizeof(Vector2), true);
	const Vector2 raw = *reinterpret_cast<const Vector2*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw, sizeof(float));
}

Vector3 Pu::BinaryReader::PeekVector3(void) const
{
	CheckOverflow(sizeof(Vector3), true);
	const Vector3 raw = *reinterpret_cast<const Vector3*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw, sizeof(float));
}

Vector4 Pu::BinaryReader::PeekVector4(void) const
{
	CheckOverflow(sizeof(Vector4), true);
	const Vector4 raw = *reinterpret_cast<const Vector4*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw, sizeof(float));
}

Quaternion Pu::BinaryReader::PeekQuaternion(void) const
{
	CheckOverflow(sizeof(Quaternion), true);
	const Quaternion raw = *reinterpret_cast<const Quaternion*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw, sizeof(float));
}

Quaternion Pu::BinaryReader::PeekPackedQuaternion(void) const
{
	return Quaternion::Unpack(PeekInt64());
}

Matrix Pu::BinaryReader::PeekMatrix(void) const
{
	CheckOverflow(sizeof(Matrix), true);
	const Matrix raw = *reinterpret_cast<const Matrix*>(data + position);
	return endian == NativeEndian ? raw : ByteSwap(raw, sizeof(float));
}

string Pu::BinaryReader::PeekString(void) const
{
	/* Start be peeking the length. */
	CheckOverflow(sizeof(string_length_t), true);
	string_length_t len = *reinterpret_cast<const string_length_t*>(data + position);
	if (endian != NativeEndian) len = ByteSwap(len);

	/* Check the required length again, make sure to add the length as well. */
	CheckOverflow(len + sizeof(string_length_t), true);

	/* Create the result string with the correct length. */
	string result;
	result.reserve(len);

	/* Add the characters individually. */
	for (size_t i = 0, start = position + sizeof(string_length_t); i < len; i++)
	{
		result += *reinterpret_cast<const char*>(data + start + i);
	}

	return result;
}

ustring Pu::BinaryReader::PeekUString(void) const
{
	/* Start be peeking the length. */
	CheckOverflow(sizeof(string_length_t), true);
	string_length_t len = *reinterpret_cast<const string_length_t*>(data + position);
	if (endian != NativeEndian) len = ByteSwap(len);

	/* Check the required length again, make sure to add the length as well. */
	const size_t byteSize = len * sizeof(char32);
	CheckOverflow(byteSize + sizeof(string_length_t), true);

	/* Create the result string with the correct length. */
	ustring result;
	result.reserve(len);

	/* Add the characters individually. */
	for (size_t i = 0, start = position + sizeof(string_length_t); i < byteSize; i += sizeof(char32))
	{
		const char32 c = *reinterpret_cast<const char32*>(data + start + i);
		result += endian == NativeEndian ? c : ByteSwap(c);
	}

	return result;
}

int32 Pu::BinaryReader::Peek(void)
{
	return CheckOverflow(1, false) ? static_cast<int32>(PeekByte()) : -1;
}

size_t Pu::BinaryReader::Peek(byte * buffer, size_t offset, size_t amount)
{
	/* Reduce amount if needed. */
	if (size - position < amount) amount = size - position;

	/* Copy data and return amount. */
	memcpy(buffer + offset, data + position, amount);
	return amount;
}

void Pu::BinaryReader::Seek(SeekOrigin from, int64 amount)
{
	int64 newPos;

	/* Get new position. */
	switch (from)
	{
	case SeekOrigin::Begin:
		newPos = amount;
		break;
	case SeekOrigin::Current:
		newPos = static_cast<int64>(position) + amount;
		break;
	case SeekOrigin::End:
		newPos = static_cast<int64>(size) - amount;
		break;
	default:
		Log::Error("BinaryReader doesn't support seek origin!");
		return;
	}

	/* Check new position and set if correct. */
	if (newPos < 0 || newPos > static_cast<int64>(size)) Log::Fatal("Cannot seek outside of buffer size!");
	position = static_cast<size_t>(newPos);
}

bool Pu::BinaryReader::CheckOverflow(size_t bytesNeeded, bool raise) const
{
	if (size - position < bytesNeeded)
	{
		if (raise) Log::Fatal("Cannot read past buffer size!");
		return false;
	}

	return true;
}

bool Pu::BinaryReader::ReadBool(void)
{
	const bool result = PeekBool();
	position += sizeof(bool);
	return result;
}

byte Pu::BinaryReader::ReadByte(void)
{
	const byte result = PeekByte();
	position += sizeof(byte);
	return result;
}

int8 Pu::BinaryReader::ReadSByte(void)
{
	const int8 result = PeekSByte();
	position += sizeof(int8);
	return result;
}

int16 Pu::BinaryReader::ReadInt16(void)
{
	const int16 result = PeekInt16();
	position += sizeof(int16);
	return result;
}

uint16 Pu::BinaryReader::ReadUInt16(void)
{
	const uint16 result = PeekUInt16();
	position += sizeof(uint16);
	return result;
}

int32 Pu::BinaryReader::ReadInt32(void)
{
	const int32 result = PeekInt32();
	position += sizeof(int32);
	return result;
}

uint32 Pu::BinaryReader::ReadUInt32(void)
{
	const uint32 result = PeekUInt32();
	position += sizeof(uint32);
	return result;
}

int64 Pu::BinaryReader::ReadInt64(void)
{
	const int64 result = PeekInt64();
	position += sizeof(int64);
	return result;
}

uint64 Pu::BinaryReader::ReadUInt64(void)
{
	const uint64 result = PeekUInt64();
	position += sizeof(uint64);
	return result;
}

float Pu::BinaryReader::ReadSingle(void)
{
	const float result = PeekSingle();
	position += sizeof(float);
	return result;
}

double Pu::BinaryReader::ReadDouble(void)
{
	const double result = PeekDouble();
	position += sizeof(double);
	return result;
}

Vector2 Pu::BinaryReader::ReadVector2(void)
{
	const Vector2 result = PeekVector2();
	position += sizeof(Vector2);
	return result;
}

Vector3 Pu::BinaryReader::ReadVector3(void)
{
	const Vector3 result = PeekVector3();
	position += sizeof(Vector3);
	return result;
}

Vector4 Pu::BinaryReader::ReadVector4(void)
{
	const Vector4 result = PeekVector4();
	position += sizeof(Vector4);
	return result;
}

Quaternion Pu::BinaryReader::ReadQuaternion(void)
{
	const Quaternion result = PeekQuaternion();
	position += sizeof(Quaternion);
	return result;
}

Quaternion Pu::BinaryReader::ReadPackedQuaternion(void)
{
	return Quaternion::Unpack(ReadInt64());
}

Matrix Pu::BinaryReader::ReadMatrix(void)
{
	const Matrix result = PeekMatrix();
	position += sizeof(Matrix);
	return result;
}

string Pu::BinaryReader::ReadString(void)
{
	const string result = PeekString();
	position += sizeof(string_length_t) + result.length();
	return result;
}

ustring Pu::BinaryReader::ReadUString(void)
{
	const ustring result = PeekUString();
	position += sizeof(string_length_t) + result.length() * sizeof(char32);
	return result;
}