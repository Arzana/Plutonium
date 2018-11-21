#include "Streams/BinaryReader.h"
#include "Core/SafeMemory.h"
#include <cstring>

using namespace Pu;

Pu::BinaryReader::BinaryReader(const void * source, size_t size)
	: data(reinterpret_cast<const byte*>(source)), size(size), position(0)
{}

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

bool Pu::BinaryReader::PeekBool(void)
{
	CheckOverflow(1, true);
	return data[position] != 0;
}

byte Pu::BinaryReader::PeekByte(void)
{
	CheckOverflow(1, true);
	return data[position];
}

int8 Pu::BinaryReader::PeekSByte(void)
{
	CheckOverflow(sizeof(int8), true);
	return *reinterpret_cast<const int8*>(void_ptr(data + position));
}

int16 Pu::BinaryReader::PeekInt16(void)
{
	CheckOverflow(sizeof(int16), true);
	return *reinterpret_cast<const int16*>(void_ptr(data + position));
}

uint16 Pu::BinaryReader::PeekUInt16(void)
{
	CheckOverflow(sizeof(uint16), true);
	return *reinterpret_cast<const uint16*>(void_ptr(data + position));
}

int32 Pu::BinaryReader::PeekInt32(void)
{
	CheckOverflow(sizeof(int32), true);
	return *reinterpret_cast<const int32*>(void_ptr(data + position));
}

uint32 Pu::BinaryReader::PeekUInt32(void)
{
	CheckOverflow(4, true);
	return *reinterpret_cast<const uint32*>(void_ptr(data + position));
}

int64 Pu::BinaryReader::PeekInt64(void)
{
	CheckOverflow(sizeof(int64), true);
	return *reinterpret_cast<const int64*>(void_ptr(data + position));
}

uint64 Pu::BinaryReader::PeekUInt64(void)
{
	CheckOverflow(sizeof(uint64), true);
	return *reinterpret_cast<const uint64*>(void_ptr(data + position));
}

float Pu::BinaryReader::PeekSingle(void)
{
	CheckOverflow(sizeof(float), true);
	return *reinterpret_cast<const float*>(void_ptr(data + position));
}

double Pu::BinaryReader::PeekDouble(void)
{
	CheckOverflow(sizeof(double), true);
	return *reinterpret_cast<const double*>(void_ptr(data + position));
}

Vector2 Pu::BinaryReader::PeekVector2(void)
{
	CheckOverflow(sizeof(Vector2), true);
	return *reinterpret_cast<const Vector2*>(void_ptr(data + position));
}

Vector3 Pu::BinaryReader::PeekVector3(void)
{
	CheckOverflow(sizeof(Vector3), true);
	return *reinterpret_cast<const Vector3*>(void_ptr(data + position));
}

Vector4 Pu::BinaryReader::PeekVector4(void)
{
	CheckOverflow(sizeof(Vector4), true);
	return *reinterpret_cast<const Vector4*>(void_ptr(data + position));
}

Quaternion Pu::BinaryReader::PeekQuaternion(void)
{
	CheckOverflow(sizeof(Quaternion), true);
	return *reinterpret_cast<const Quaternion*>(void_ptr(data + position));
}

Matrix Pu::BinaryReader::PeekMatrix(void)
{
	CheckOverflow(sizeof(Matrix), true);
	return *reinterpret_cast<const Matrix*>(void_ptr(data + position));
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

bool Pu::BinaryReader::CheckOverflow(size_t bytesNeeded, bool raise)
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

Matrix Pu::BinaryReader::ReadMatrix(void)
{
	const Matrix result = PeekMatrix();
	position += sizeof(Matrix);
	return result;
}