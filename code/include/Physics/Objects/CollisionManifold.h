#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* 
	Defines the handle to a physical object.

	Physical handles store various pieces of fast information.
	[TTA00000 00000000 IIIIIIII IIIIIIII]

	T (2-bits):		The type of the object, also the PhysicalWorld vector in which it is stored.
	A (1-bit):		Allocation flag, used by the BVH to set whether the node is in use.
	I (16-bits):	The index in the lookup vector, used to determine the actual index.
	*/
	using PhysicsHandle = uint32;
	/* Defines the handle used to denote a null handle. */
	constexpr PhysicsHandle PhysicsNullHandle = ~0u;

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