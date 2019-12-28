#pragma once
#include "AABB.h"
#include "Vector4.h"

namespace Pu
{
	/* Defines a 3D plane as a surface normal and distance to the origin. */
	struct Plane
	{
	public:
		/* The surface normal of the plane. */
		Vector3 N;
		/* The distance to the origin. */
		float D;

		/* Initializes a new instance of a plane as an invalid plane with no normal. */
		Plane(void)
			: N(), D(0.0f)
		{}

		/* Initialize a new instance of a plane with a specified surface normal and distance to the origin. */
		Plane(_In_ Vector3 normal, _In_ float dist)
			: N(normalize(normal)), D(dist)
		{}

		/* Initializes a new instance of a plane from matrix numbers. */
		Plane(_In_ Vector4 numbers)
			: N(numbers.X, numbers.Y, numbers.Z)
		{
			const float len = N.Length();

			N /= len;
			D = numbers.W / len;
		}

		/* Gets the intersection point between the 3 infinite planes. */
		_Check_return_ static inline Vector3 IntersectionPoint(_In_ const Plane &a, _In_ const Plane &b, _In_ const Plane &c)
		{
			const Vector3 v1 = a.D * cross(b.N, c.N);
			const Vector3 v2 = b.D * cross(c.N, a.N);
			const Vector3 v3 = c.D * cross(a.N, b.N);
			const float denom = -dot(a.N, cross(b.N, c.N));
			return (v1 + v2 + v3) / denom;
		}

		/* Gets the relative location of the point to the plane, positve numbers mean the point is in front of the plane, negative means it's behind and zero means it's on the plane.  */
		_Check_return_ inline float HalfSpace(_In_ Vector3 p) const
		{
			return dot(N, p) + D;
		}

		/* Checks whether the ray is intersection the plane. */
		_Check_return_ inline bool IntersectionRay(_In_ Vector3 origin, _In_ Vector3 direction) const
		{
			return (-(HalfSpace(origin) / dot(N, direction))) >= 0;
		}

		/* Gets whether the sphere is intersecting or is in front of the plane. */
		_Check_return_ inline bool IntersectionSphere(_In_ Vector3 center, _In_ float radius) const
		{
			return HalfSpace(center) >= -radius;
		}

		/* Gets whether the box is intersecting or is in front of the plane. */
		_Check_return_ inline bool IntersectionBox(_In_ const AABB &aabb) const
		{
			constexpr size_t CORNER_CNT = 8;

			size_t cnt = CORNER_CNT;
			for (size_t i = 0; i < CORNER_CNT; i++)
			{
				if (HalfSpace(aabb[i]) < 0.0f) --cnt;
			}

			return cnt > 0;
		}
	};
}