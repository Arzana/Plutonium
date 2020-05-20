#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines the handle to a physical object. */
	using PhysicsHandle = uint64;

	/* Defines the parameters needed to solve a collision. */
	struct CollisionManifold
	{
		/* The first object in the collision. */
		PhysicsHandle FirstObject;
		/* The second object in the collision. */
		PhysicsHandle SecondObject;
		/* The normal at the collision point. */
		Vector3 N;
	};
}