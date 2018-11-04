#include "Streams\BinaryReader.h"
#include "Core\SafeMemory.h"
#include <cstring>

using namespace Plutonium;

Plutonium::BinaryReader::BinaryReader(const void * source, size_t size)
	: data(reinterpret_cast<const byte*>(source)), size(size), position(0)
{}

int32 Plutonium::BinaryReader::Read(void)
{
	return CheckOverflow(1, false) ? static_cast<int32>(ReadByte()) : -1;
}

size_t Plutonium::BinaryReader::Read(byte * buffer, size_t offset, size_t amount)
{
	const size_t result = Peek(buffer, offset, amount);
	position += result;
	return result;
}

bool Plutonium::BinaryReader::PeekBool(void)
{
	CheckOverflow(1, true);
	return data[position] != 0;
}

byte Plutonium::BinaryReader::PeekByte(void)
{
	CheckOverflow(1, true);
	return data[position];
}

int8 Plutonium::BinaryReader::PeekSByte(void)
{
	CheckOverflow(sizeof(int8), true);
	return *reinterpret_cast<const int8*>(void_ptr(data + position));
}

int16 Plutonium::BinaryReader::PeekInt16(void)
{
	CheckOverflow(sizeof(int16), true);
	return *reinterpret_cast<const int16*>(void_ptr(data + position));
}

uint16 Plutonium::BinaryReader::PeekUInt16(void)
{
	CheckOverflow(sizeof(uint16), true);
	return *reinterpret_cast<const uint16*>(void_ptr(data + position));
}

int32 Plutonium::BinaryReader::PeekInt32(void)
{
	CheckOverflow(sizeof(int32), true);
	return *reinterpret_cast<const int32*>(void_ptr(data + position));
}

uint32 Plutonium::BinaryReader::PeekUInt32(void)
{
	CheckOverflow(4, true);
	return *reinterpret_cast<const uint32*>(void_ptr(data + position));
}

int64 Plutonium::BinaryReader::PeekInt64(void)
{
	CheckOverflow(sizeof(int64), true);
	return *reinterpret_cast<const int64*>(void_ptr(data + position));
}

uint64 Plutonium::BinaryReader::PeekUInt64(void)
{
	CheckOverflow(sizeof(uint64), true);
	return *reinterpret_cast<const uint64*>(void_ptr(data + position));
}

float Plutonium::BinaryReader::PeekSingle(void)
{
	CheckOverflow(sizeof(float), true);
	return *reinterpret_cast<const float*>(void_ptr(data + position));
}

double Plutonium::BinaryReader::PeekDouble(void)
{
	CheckOverflow(sizeof(double), true);
	return *reinterpret_cast<const double*>(void_ptr(data + position));
}

Vector2 Plutonium::BinaryReader::PeekVector2(void)
{
	CheckOverflow(sizeof(Vector2), true);
	return *reinterpret_cast<const Vector2*>(void_ptr(data + position));
}

Vector3 Plutonium::BinaryReader::PeekVector3(void)
{
	CheckOverflow(sizeof(Vector3), true);
	return *reinterpret_cast<const Vector3*>(void_ptr(data + position));
}

Vector4 Plutonium::BinaryReader::PeekVector4(void)
{
	CheckOverflow(sizeof(Vector4), true);
	return *reinterpret_cast<const Vector4*>(void_ptr(data + position));
}

Quaternion Plutonium::BinaryReader::PeekQuaternion(void)
{
	CheckOverflow(sizeof(Quaternion), true);
	return *reinterpret_cast<const Quaternion*>(void_ptr(data + position));
}

Matrix Plutonium::BinaryReader::PeekMatrix(void)
{
	CheckOverflow(sizeof(Matrix), true);
	return *reinterpret_cast<const Matrix*>(void_ptr(data + position));
}

int32 Plutonium::BinaryReader::Peek(void)
{
	return CheckOverflow(1, false) ? static_cast<int32>(PeekByte()) : -1;
}

size_t Plutonium::BinaryReader::Peek(byte * buffer, size_t offset, size_t amount)
{
	/* Reduce amount if needed. */
	if (size - position < amount) amount = size - position;

	/* Copy data and return amount. */
	memcpy(buffer + offset, data + position, amount);
	return amount;
}

void Plutonium::BinaryReader::Seek(SeekOrigin from, int64 amount)
{
	int64 newPos = position;

	/* Get new position. */
	switch (from)
	{
	case Plutonium::SeekOrigin::Begin:
		newPos = amount;
		break;
	case Plutonium::SeekOrigin::Current:
		newPos = static_cast<int64>(position) + amount;
		break;
	case Plutonium::SeekOrigin::End:
		newPos = static_cast<int64>(size) - amount;
		break;
	default:
		LOG_THROW("BinaryReader doesn't support seek origin!");
#if defined(DEBUG)
		break;
#endif
	}

	/* Check new position and set if correct. */
	LOG_THROW_IF(newPos < 0 || newPos > static_cast<int64>(size), "Cannot seek outside of buffer size!");
	position = static_cast<size_t>(newPos);
}

bool Plutonium::BinaryReader::CheckOverflow(size_t bytesNeeded, bool raise)
{
	if (size - position < bytesNeeded)
	{
		LOG_THROW_IF(raise, "Cannot read past buffer size!");
		return false;
	}

	return true;
}

bool Plutonium::BinaryReader::ReadBool(void)
{
	bool result = PeekBool();
	position += 1;
	return result;
}

byte Plutonium::BinaryReader::ReadByte(void)
{
	byte result = PeekByte();
	position += sizeof(byte);
	return result;
}

int8 Plutonium::BinaryReader::ReadSByte(void)
{
	int8 result = PeekSByte();
	position += sizeof(int8);
	return result;
}

int16 Plutonium::BinaryReader::ReadInt16(void)
{
	int16 result = PeekInt16();
	position += sizeof(int16);
	return result;
}

uint16 Plutonium::BinaryReader::ReadUInt16(void)
{
	uint16 result = PeekUInt16();
	position += sizeof(uint16);
	return result;
}

int32 Plutonium::BinaryReader::ReadInt32(void)
{
	int32 result = PeekInt32();
	position += sizeof(int32);
	return result;
}

uint32 Plutonium::BinaryReader::ReadUInt32(void)
{
	uint32 result = PeekUInt32();
	position += sizeof(uint32);
	return result;
}

int64 Plutonium::BinaryReader::ReadInt64(void)
{
	int64 result = PeekInt64();
	position += sizeof(int64);
	return result;
}

uint64 Plutonium::BinaryReader::ReadUInt64(void)
{
	uint64 result = PeekUInt64();
	position += sizeof(uint64);
	return result;
}

float Plutonium::BinaryReader::ReadSingle(void)
{
	float result = PeekSingle();
	position += sizeof(float);
	return result;
}

double Plutonium::BinaryReader::ReadDouble(void)
{
	double result = PeekDouble();
	position += sizeof(double);
	return result;
}

Vector2 Plutonium::BinaryReader::ReadVector2(void)
{
	Vector2 result = PeekVector2();
	position += sizeof(Vector2);
	return result;
}

Vector3 Plutonium::BinaryReader::ReadVector3(void)
{
	Vector3 result = PeekVector3();
	position += sizeof(Vector3);
	return result;
}

Vector4 Plutonium::BinaryReader::ReadVector4(void)
{
	Vector4 result = PeekVector4();
	position += sizeof(Vector4);
	return result;
}

Quaternion Plutonium::BinaryReader::ReadQuaternion(void)
{
	Quaternion result = PeekQuaternion();
	position += sizeof(Quaternion);
	return result;
}

Matrix Plutonium::BinaryReader::ReadMatrix(void)
{
	Matrix result = PeekMatrix();
	position += sizeof(Matrix);
	return result;
}
