#pragma once
#include <sal.h>

namespace Pu
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

	/* Gets a human readable version of the seek origin enum. */
	_Check_return_ inline const char* to_string(_In_ SeekOrigin value)
	{
		switch (value)
		{
		case SeekOrigin::Begin:
			return "Begin";
		case SeekOrigin::Current:
			return "Current";
		case SeekOrigin::End:
			return "End";
		default:
			return "Unknown";
		}
	}
}