#pragma once
#include "Core/Math/Vector3.h"

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

		/* Initializes a new instance of a sphere. */
		Sphere(_In_ Vector3 center, _In_ float radius)
			: Center(center), Radius(radius)
		{}
	};
}