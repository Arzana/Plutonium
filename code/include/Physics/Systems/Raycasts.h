#pragma once
#include "Core/Math/Shapes/AABB.h"
#include "Core/Math/Shapes/Plane.h"

namespace Pu
{
	/*
	Gets the distance on the ray at which the ray intersects with the plane, if it doesn't intersect; the value is negative.
	Note that the ray is specified as it's starting position (p) and its direction (d).
	*/
	_Check_return_ inline float raycast(_In_ Vector3 p, _In_ Vector3 d, _In_ Plane plane)
	{
		return -(halfspace(plane, p) / dot(plane.N, d));
	}

	/* 
	Gets the distance on the ray at which the ray intersects with the axis aligned bounding box, if it doesn't intersect; the value is negative. 
	Note that the ray is specified as it's starting position (p) and the reciprocal of its direction (rd).
	*/
	_Check_return_ inline float raycast(_In_ Vector3 p, _In_ Vector3 rd, _In_ const AABB &box)
	{
		const float t[] =
		{
			(box.LowerBound.X - p.X) * rd.X,
			(box.UpperBound.X - p.X) * rd.X,
			(box.LowerBound.Y - p.Y) * rd.Y,
			(box.UpperBound.Y - p.Y) * rd.Y,
			(box.LowerBound.Z - p.Z) * rd.Z
		};

		const float mi = max(max(min(t[0], t[1]), min(t[2], t[3])), min(t[4], t[5]));
		const float ma = min(min(max(t[0], t[1]), max(t[2], t[3])), max(t[4], t[5]));

		if (ma < 0.0f || mi > ma) return -1.0f;
		return mi < 0.0f ? ma : mi;
	}
}