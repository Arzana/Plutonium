#pragma once
#include "CollisionShapes.h"
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines the handle for a collider. */
	using ColliderHndl = void*;
	/* Defines the event handler signature for a collision event. */
	using CollisionEventHandler = void(*)(_In_ ColliderHndl hndl);

	/* Defines the information about a sphere collider. */
	struct SphereColliderInfo
	{
		/* Defines the type of collider (always Sphere). */
		CollisionShapes Type;
		/* Defines the center of the sphere. */
		Vector3 Center;
		/* Defines the radius of the sphere. */
		float Radius;

		/* Initializes a new intance of a sphere collider information object. */
		SphereColliderInfo(_In_ Vector3 center, _In_ float radius)
			: Type(CollisionShapes::Sphere), Center(center), Radius(radius)
		{}
	};

	/* Defines the information needed to create a collider. */
	struct ColliderCreateInfo
	{
	public:
		/* Defines the information about the broad phase shape. */
		const void *BroadShapeInfo;
		/* Defines the information about the narrow phase shape. */
		const void *NarrowShapeInfo;
		/* Defines an optional identifier from the caller for this object. */
		void *UserIdentifier;
		/* Defines an optional event handler for collision events. */
		CollisionEventHandler EventHandler;

		/* Initializes a new instance of a collider creation information object. */
		ColliderCreateInfo(_In_ const void *broadPhase, _In_ const void *narrowPhase)
			: BroadShapeInfo(broadPhase), NarrowShapeInfo(narrowPhase),
			UserIdentifier(nullptr), EventHandler(nullptr)
		{}
	};
}