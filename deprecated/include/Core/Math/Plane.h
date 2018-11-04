#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace Plutonium
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

			N.Normalize();
			D = numbers.W / len;
		}

		/* Gets the relative location of the point to the plane, positve numbers mean the point is in front of the plane, negative means it's behind and zero means it's on the plane.  */
		_Check_return_ inline float HalfSpace(_In_ Vector3 p) const
		{
			return dot(N, p) + D;
		}

		/* Checks whether the ray is intersection the plane. */
		_Check_return_ inline bool IntersectionRay(_In_ Vector3 origin, _In_ Vector3 direction)
		{
			return (-(HalfSpace(origin) / dot(N, direction))) >= 0;
		}

		/* Gets whether the sphere is intersecting or is in front of the plane. */
		_Check_return_ inline bool IntersectionSphere(_In_ Vector3 center, _In_ float radius) const
		{
			return HalfSpace(center) >= -radius;
		}

		/* Gets whether the box is intersecting or is in front of the plane. */
		_Check_return_ inline bool IntersectionBox(_In_ Vector3 pos, _In_ Vector3 size) const
		{
			constexpr size_t CORNER_CNT = 8;

			Vector3 corners[CORNER_CNT] =
			{
				pos,
				pos + Vector3(size.X, 0.0f, 0.0f),
				pos + Vector3(0.0f, size.Y, 0.0f),
				pos + Vector3(0.0f, 0.0f, size.Z),
				pos + Vector3(size.X, size.Y, 0.0f),
				pos + Vector3(size.X, 0.0f, size.Z),
				pos + Vector3(0.0f, size.Y, size.Z),
				pos + size
			};

			size_t cnt = CORNER_CNT;
			for (size_t i = 0; i < CORNER_CNT; i++)
			{
				if (HalfSpace(corners[i]) < 0.0f) --cnt;
			}

			return cnt > 0;
		}
	};
}