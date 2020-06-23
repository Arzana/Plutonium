#pragma once
#include "PointTests.h"
#include "Core/Math/Shapes/Frustum.h"

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

	/*
	Gets whether the specified axis aligned bounding box intersects with the specified plane. 
	Note that this also return true if the axis aligned bounding box is in front of the plane.
	*/
	_Check_return_ inline bool intersects_or_front(_In_ const Vector3 corners[8], _In_ Plane plane)
	{
		uint8 cnt = 8;
		for (uint8 i = 0; i < 8; i++)
		{
			cnt -= halfspace(plane, corners[i]) < 0.0f;
		}

		return cnt > 0;
	}

	/*
	Gets whether the specified sphere intersects with the specified plane.
	Note that this also return true if the sphere is in front of the plane.
	*/
	_Check_return_ inline bool intersects_or_front(_In_ Sphere sphere, _In_ Plane plane)
	{
		return halfspace(plane, sphere.Center) >= -sphere.Radius;
	}

	/* Gets whether the specified sphere intersects with the specified frustum. */
	_Check_return_ inline bool intersects(_In_ const Frustum &frustum, _In_ Sphere sphere)
	{
		for (uint8 i = 0; i < 6; i++)
		{
			if (!intersects_or_front(sphere, frustum.Planes[i])) return false;
		}

		return true;
	}

	/* Gets whether the specified axis aligned bounding box intersects with the specified frustum. */
	_Check_return_ inline bool intersects(_In_ const Frustum &frustum, _In_ const AABB &box)
	{
		/* Precalculate all the corners of the axis aligned bounding box. */
		const Vector3 corners[8] =
		{
			box.LowerBound,
			Vector3(box.UpperBound.X, box.LowerBound.Y, box.LowerBound.Z),
			Vector3(box.UpperBound.X, box.UpperBound.Y, box.LowerBound.Z),
			Vector3(box.LowerBound.X, box.UpperBound.Y, box.LowerBound.Z),
			Vector3(box.LowerBound.X, box.UpperBound.Y, box.UpperBound.Z),
			Vector3(box.LowerBound.X, box.LowerBound.Y, box.UpperBound.Z),
			Vector3(box.UpperBound.X, box.LowerBound.Y, box.UpperBound.Z),
			box.UpperBound
		};

		/* 
		Check the corners agains all the planes of the frustum.
		If it's 'outside' of one of the planes then it doesn't intersect.
		*/
		for (uint8 i = 0; i < 6; i++)
		{
			if (!intersects_or_front(corners, frustum.Planes[i])) return false;
		}

		/* The axis aligned bounding box is either fully inside the frustum or it's intersection one or more planes. */
		return true;
	}
}