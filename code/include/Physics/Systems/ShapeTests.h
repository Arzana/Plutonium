#pragma once
#include "PointTests.h"

namespace Pu
{
	/* Gets whether the two specified spheres intersect. */
	_Check_return_ inline bool intersects(_In_ Sphere s1, _In_ Sphere s2)
	{
		return sqrdist(s1.Center, s2.Center) < sqr(s1.Radius + s2.Radius);
	}

	/* Gets whether the specified sphere intersects with the specified axis aligned bounding box. */
	_Check_return_ inline bool intersects(_In_ Sphere sphere, _In_ const AABB &box)
	{
		return sqrdist(closest(box, sphere.Center), sphere.Center) < sqr(sphere.Radius);
	}

	/* Gets whether the specified sphere intersects with the specified oriented bounding box. */
	_Check_return_ inline bool intersects(_In_ Sphere sphere, _In_ const OBB &box)
	{
		return sqrdist(closest(box, sphere.Center), sphere.Center) < sqr(sphere.Radius);
	}

	/* Gets whether the specified sphere intersects with the specified plane. */
	_Check_return_ inline bool intersects(_In_ Sphere sphere, _In_ Plane plane)
	{
		return sqrdist(closest(plane, sphere.Center), sphere.Center) < sqr(sphere.Radius);
	}

	/* Gets whether the two specified axis aligned bounding boxes intersect. */
	_Check_return_ inline bool intersects(_In_ const AABB &b1, _In_ const AABB &b2)
	{
		return b1.LowerBound.X <= b2.UpperBound.X && b1.UpperBound.X >= b2.LowerBound.X
			&& b1.LowerBound.Y <= b2.UpperBound.Y && b1.UpperBound.Y >= b2.LowerBound.Y
			&& b1.UpperBound.Z <= b2.UpperBound.Z && b1.UpperBound.Z >= b2.LowerBound.Z;
	}
}