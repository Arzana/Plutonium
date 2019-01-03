#pragma once
#include "BinaryReader.h"

namespace Pu
{
	/* Defines an object that can read individual bits from a binary source. */
	class BitStreamReader
	{
	public:
		/* Initializes a new instance of a bit reader from binary reader. */
		BitStreamReader(_In_ const BinaryReader &source);
		/* Initialize a new instance of a bit reader form a binary source. */
		BitStreamReader(_In_ const void *source, _In_ size_t size, _In_opt_ Endian endian = BinaryReader::NativeEndian);
		/* Copy constructor. */
		BitStreamReader(_In_ const BitStreamReader &value);
		/* Move constructor. */
		BitStreamReader(_In_ BitStreamReader &&value);

		/* Copy assignment. */
		_Check_return_ BitStreamReader& operator =(_In_ const BitStreamReader &other);
		/* Move assignment. */
		_Check_return_ BitStreamReader& operator =(_In_ BitStreamReader &&other);

		/* Reads the next bit from the stream and advances the read position. */
		_Check_return_ inline bool ReadBit(void)
		{
			return MaskedRead(0x1, 1);
		}
		/* Reads the next 2 bits from the stream and advances the read position. */
		_Check_return_ inline byte ReadCrumb(void)
		{
			return MaskedRead(0x3, 2);
		}
		/* Reads the next 4 bits from the stream and advances the read position. */
		_Check_return_ inline byte ReadNibble(void)
		{
			return MaskedRead(0xF, 4);
		}
		/* Reads the next 8 bits from the stream and advances the read position. */
		_Check_return_ inline byte ReadByte(void)
		{
			return MaskedRead(0xFF, 8);
		}

		/* Reads a generic amount (1 to 8) of bits from the stream and advances the read position. */
		template <size_t _Amount>
		_Check_return_ inline byte Read(void)
		{
			static_assert(_Amount > 0 && _Amount <= 8, "BitStreamReader generic read invalid range!");
			return MaskedRead(0xFF >> (CHAR_BIT - _Amount), static_cast<uint16>(_Amount));
		}

		/* Reads the next bit from the stream. */
		_Check_return_ inline bool PeekBit(void) const
		{
			return MaskedPeek(0x1);
		}

		/* Reads the next 2 bits from the stream. */
		_Check_return_ inline byte PeekCrumb(void) const
		{
			return MaskedPeek(0x3);
		}

		/* Reads the next 4 bits from the stream. */
		_Check_return_ inline byte PeekNibble(void) const
		{
			return MaskedPeek(0xF);
		}

		/* Reads the next 8 bits from the stream. */
		_Check_return_ inline byte PeekByte(void) const
		{
			return MaskedPeek(0xFF);
		}

		/* Peeks a generic amount (1 to 8) of bits from the stream. */
		template <size_t _Amount>
		_Check_return_ inline byte Peek(void) const
		{
			static_assert(_Amount > 0 && _Amount <= 8, "BitStreamReader generic peek invalid range!");
			return MaskedPeek(0xFF >> (CHAR_BIT - _Amount));
		}

		/* Gets the underlying binary reader. */
		_Check_return_ inline BinaryReader& GetReader(void)
		{
			return reader;
		}

		/* Discard all bits untill the next byte boundry is reached. */
		void DiscardBits(void);

	private:
		BinaryReader reader;
		uint16 stored, position;
		bool endReached;

		byte MaskedPeek(byte mask) const;
		byte MaskedRead(byte mask, uint16 advance);
		void ReadIfNeeded(void);
		void Initialize(void);
	};
}