#pragma once
#include "CollisionManifold.h"
#include "Core/Events/EventBus.h"
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
		/* Specifies an optional unique indentifier which can be used to indentify the object. */
		const void *UserParam;
		/* Occurs just before the collision on this object is resolved. */
		EventBus<const void*, const CollisionManifold&> OnCollision;

		/* Initializes a new instance of a collider. */
		Collider(_In_ AABB broad, _In_ CollisionShapes narrow, _In_ void *params)
			: BroadPhase(broad), NarrowPhaseShape(narrow), NarrowPhaseParameters(params),
			UserParam(nullptr), OnCollision("Collider::OnCollision")
		{}
	};
}