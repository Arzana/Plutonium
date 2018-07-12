#pragma once

namespace Plutonium
{
	/* Defines from where a stream should seek. */
	enum class SeekOrigin : int
	{
		/* The beginning of the stream. */
		Begin = 0,
		/* The current position of the stream. */
		Current = 1,
		/* The end of the stream. */
		End = 2
	};
}