#pragma once
#include "Core/Math/Shapes/AABB.h"
#include "Physics/Properties/CollisionShapes.h"

namespace Pu
{
	/* Defines a collider that can be used in the physical world. */
	class Collider
	{
	public:
		/* Specifies an axis-aligned bounding box used for broad phase collision detection. */
		AABB BroadPhase;
		/* Specifies the type of narrow phase collision shape to use. */
		CollisionShapes NarrowPhaseShape;
		/* Specifies the parameters for the narrow phase collider (null for None). */
		void *NarrowPhaseParameters;

		/* Initializes an empty instance of a collider. */
		Collider(void)
			: NarrowPhaseShape(CollisionShapes::None),
			NarrowPhaseParameters(nullptr)
		{}

		/* Initializes a new instance of a collider. */
		Collider(_In_ AABB broad, _In_ CollisionShapes narrow, _In_ void *params)
			: BroadPhase(broad), NarrowPhaseShape(narrow), NarrowPhaseParameters(params)
		{}
	};
}