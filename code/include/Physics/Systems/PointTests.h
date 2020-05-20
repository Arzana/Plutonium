#pragma once
#include "Core/Math/Shapes/Sphere.h"
#include "Core/Math/Shapes/AABB.h"
#include "Core/Math/Shapes/OBB.h"
#include "Core/Math/Shapes/Plane.h"
#include "Core/Math/Shapes/Line.h"

namespace Pu
{
	/* Gets the closest point on the specified sphere to the specified point. */
	_Check_return_ inline Vector3 closest(_In_ Sphere sphere, _In_ Vector3 point)
	{
		return sphere.Center + dir(sphere.Center, point) * sphere.Radius;
	}

	/* Gets the closest point in the specified axis aligned bounding box to the specified point. */
	_Check_return_ inline Vector3 closest(_In_ const AABB &box, _In_ Vector3 point)
	{
		return clamp(point, box.LowerBound, box.UpperBound);
	}

	/* Gets the closest point in the specified oriented bounding box to the specified point. */
	_Check_return_ inline Vector3 closest(_In_ const OBB &box, _In_ Vector3 point)
	{
		const Vector3 delta = point - box.Center;
		const float dx = clamp(dot(delta, box.GetRight()), -box.Extent.X, box.Extent.X);
		const float dy = clamp(dot(delta, box.GetUp()), -box.Extent.Y, box.Extent.Y);
		const float dz = clamp(dot(delta, box.GetForward()), -box.Extent.Z, box.Extent.Z);

		return box.Center + dx * box.GetRight() + dy * box.GetUp() + dz * box.GetForward();
	}

	/* Gets the closest point on the specified plane to the specified point. */
	_Check_return_ inline Vector3 closest(_In_ Plane plane, _In_ Vector3 point)
	{
		return point - plane.N * (dot(plane.N, point) - plane.D);
	}

	/* Gets the closest point on the specified line to the specified point. */
	_Check_return_ inline Vector3 closest(_In_ const Line &line, _In_ Vector3 point)
	{
		const Vector3 delta = line.End - line.Start;
		return line.Start + delta * saturate(dot(point - line.Start, delta) / delta.LengthSquared());
	}

	/* Gets whether the specified point is within the bounds of the specified sphere. */
	_Check_return_ inline bool contains(_In_ Sphere sphere, _In_ Vector3 point)
	{
		return sqrdist(sphere.Center, point) < sqr(sphere.Radius);
	}

	/* Gets whether the specified point is within the specified axis aligned bounding box. */
	_Check_return_ inline bool contains(_In_ const AABB &box, _In_ Vector3 point)
	{
		return box.LowerBound.X <= point.X && box.UpperBound.X >= point.X
			&& box.LowerBound.Y <= point.Y && box.UpperBound.Y >= point.Y
			&& box.UpperBound.Z <= point.Z && box.UpperBound.Z >= point.Z;
	}

	/* Gets whether the specified point is within the specified oriented bounding box. */
	_Check_return_ inline bool contains(_In_ const OBB &box, _In_ Vector3 point)
	{
		const Vector3 delta = point - box.Center;
		const float dx = dot(delta, box.GetRight());
		const float dy = dot(delta, box.GetUp());
		const float dz = dot(delta, box.GetForward());

		return (dx >= -box.Extent.X && dx <= box.Extent.X)
			&& (dy >= -box.Extent.Y && dy <= box.Extent.Y)
			&& (dz >= -box.Extent.Z && dz <= box.Extent.Z);
	}

	/* Gets whether the specified point is on the specified plane. */
	_Check_return_ inline bool contains(_In_ Plane plane, _In_ Vector3 point, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyeql(plane.HalfSpace(point), 0.0f, tolerance);
	}

	/* Gets whether the specified point is on the specified line. */
	_Check_return_ inline bool contains(_In_ const Line &line, _In_ Vector3 point, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyeql(sqrdist(point, closest(line, point)), 0.0f, tolerance);
	}
}