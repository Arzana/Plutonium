#pragma once
#include "Core/Math/Vector3.h"

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

		/* Gets whether the specified point is on the line segment. */
		_Check_return_ inline bool Contains(_In_ Vector3 p, _In_ float tolerance = EPSILON) const
		{
			return nrlyeql(dist(Start, p) + dist(p, End), dist(Start, End), tolerance);
		}
	};

	/* Gets the direction from the start of the line to the end. */
	_Check_return_ inline Vector3 dir(_In_ const Line &line)
	{
		return normalize(line.End - line.Start);
	}
}