#include "Streams/BitStreamReader.h"
#include "Core/Diagnostics/Logging.h"

Pu::BitStreamReader::BitStreamReader(const BinaryReader & source)
	: reader(source), position(0)
{
	/* Throw a nice error if needed. */
	if (reader.GetSize() < 2) Log::Fatal("BitStreamReader needs a source with at least 2 bytes!");
	Initialize();
}

Pu::BitStreamReader::BitStreamReader(const void * source, size_t size, Endian endian)
	: reader(source, size, endian), position(0)
{
	/* Throw a nice error if needed. */
	if (size < 2) Log::Fatal("BitStreamReader needs a source with at least 2 bytes!");
	Initialize();
}

Pu::BitStreamReader::BitStreamReader(const BitStreamReader & value)
	: reader(value.reader), position(value.position), stored(value.stored)
{}

Pu::BitStreamReader::BitStreamReader(BitStreamReader && value)
	: reader(std::move(value.reader)), position(value.position), stored(value.stored)
{}

Pu::BitStreamReader & Pu::BitStreamReader::operator=(const BitStreamReader & other)
{
	if (this != &other)
	{
		reader = other.reader;
		position = other.position;
		stored = other.stored;
	}

	return *this;
}

Pu::BitStreamReader & Pu::BitStreamReader::operator=(BitStreamReader && other)
{
	if (this != &other)
	{
		reader = std::move(other.reader);
		position = other.position;
		stored = other.stored;
	}

	return *this;
}

void Pu::BitStreamReader::DiscardBits(void)
{
	if (position > 0)
	{
		position = 8;
		ReadIfNeeded();
	}
}

Pu::byte Pu::BitStreamReader::MaskedPeek(byte mask) const
{
	/* Mask the bits needed at the specific position and return them as if they were at the front. */
	return (stored & (mask << position)) >> position;
}

Pu::byte Pu::BitStreamReader::MaskedRead(byte mask, uint16 advance)
{
	const byte result = MaskedPeek(mask);
	position += advance;
	ReadIfNeeded();
	return result;
}

void Pu::BitStreamReader::ReadIfNeeded(void)
{
	/* Only read a new byte if we moved into the new byte. */
	if (position >= CHAR_BIT)
	{
		/* Throw error if the end has been reached. */
		if (endReached && position >= (CHAR_BIT << 1)) Log::Fatal("End of binary stream reached!");

		/* Only read if the end has not yet been reached. */
		if (reader.GetLocation() + 1 < reader.GetSize())
		{
			/* Move new byte into old byte. */
			stored >>= CHAR_BIT;

			/* Read new byte. */
			stored |= (reader.ReadByte() << CHAR_BIT);

			/* The position is in bits so rewind a byte. */
			position -= CHAR_BIT;
		}
		else endReached = true;
	}
}

void Pu::BitStreamReader::Initialize(void)
{
	/* Read the initial 2 bytes. */
	stored = reader.ReadByte();
	stored |= (reader.ReadByte() << CHAR_BIT);
}