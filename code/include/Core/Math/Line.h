#pragma once
#include "Vector3.h"

namespace Pu
{
	/* Defines a line segment with a starting location and an end location. */
	struct Line
	{
		/* Specifies the start location of the line. */
		Vector3 Start;
		/* Specifies the end location of the line. */
		Vector3 End;

		/* Initializes an empty instance of a line segment. */
		Line(void)
		{}

		/* Initializes a new instance of a line segment with a specific start and end point. */
		Line(_In_ Vector3 start, _In_ Vector3 end)
			: Start(start), End(end)
		{}

		/* Checks whether two line segments are equal. */
		_Check_return_ inline bool operator ==(_In_ const Line &other) const
		{
			return other.Start == Start && other.End == End;
		}

		/* Checks whether two line segments differ. */
		_Check_return_ inline bool operator !=(_In_ const Line &other) const
		{
			return other.Start != Start || other.End != End;
		}

		/* Gets the delta between the start and the endpoint. */
		_Check_return_ inline Vector3 Delta(void) const
		{
			return End - Start;
		}
	};

	/* Gets the direction from a to b. */
	_Check_return_ inline Vector3 dir(_In_ const Line &line)
	{
		return normalize(line.Delta());
	}
}