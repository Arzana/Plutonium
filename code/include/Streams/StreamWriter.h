#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines a base object for writing raw bytes. */
	class StreamWriter
	{
	public:
		/* Specifies whether the stream writer should flush after every write. */
		bool AutoFlush;

		StreamWriter(_In_ const StreamWriter&) = delete;
		StreamWriter(_In_ StreamWriter&&) = delete;
		/* Releases the resources allocated by the stream writer. */
		virtual ~StreamWriter(void) noexcept
		{}

		_Check_return_ StreamWriter& operator =(const StreamWriter&) = delete;
		_Check_return_ StreamWriter& operator =(StreamWriter&&) = delete;

		/* Flushes the stream, writing the data into the underlying stream. */
		virtual void Flush(void) = 0;
		/*Writes a single byte to the stream. */
		virtual void Write(_In_ byte value) = 0;
		/* Writes a specific range of bytes to the stream. */
		virtual void Write(_In_ const byte *data, _In_ size_t offset, _In_ size_t amount) = 0;

	protected:
		/* Initializes a new instance of a stream writer. */
		StreamWriter(void)
			: AutoFlush(false)
		{}
	};
}