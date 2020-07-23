#pragma once
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a 3D spheroid. */
	struct Sphere
	{
	public:
		/* Defines the center of the sphere. */
		Vector3 Center;
		/* Defines the radius of the sphere. */
		float Radius;

		/* Initializes an empty instance of a sphere. */
		Sphere(void)
			: Radius(0.0f)
		{}

		/* Initializes a new instance of a sphere at the origin. */
		Sphere(_In_ float radius)
			: Radius(radius)
		{}

		/* Initializes a new instance of a sphere. */
		Sphere(_In_ Vector3 center, _In_ float radius)
			: Center(center), Radius(radius)
		{}

		/* Transforms the sphere with the specified transform. */
		_Check_return_ inline Sphere operator *(_In_ const Matrix &transform) const
		{
			const Vector3 center = transform * Center;
			const Vector3 scale2 = transform.GetSquaredScale();
			const float scale = sqrtf(max(scale2.X, scale2.Y, scale2.Z));
			return Sphere{ center, Radius * scale };
		}
	};

	/* Defines the GJK support function for a sphere. */
	_Check_return_ inline Vector3 gjk_support_sphere(_In_ Vector3 dir, _In_ const void *userParam)
	{
		const Sphere *sphere = reinterpret_cast<const Sphere*>(userParam);
		return sphere->Center + dir * sphere->Radius;
	}
}