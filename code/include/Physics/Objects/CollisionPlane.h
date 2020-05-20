#pragma once
#include "Core/Events/EventBus.h"
#include "Core/Math/Shapes/Plane.h"
#include "Physics/Properties/PassOptions.h"

namespace Pu
{
	/* Defines an infinite plane of collision. */
	class CollisionPlane
	{
	public:
		/* Specifies the mathematical plane. */
		Plane Plane;
		/* Specifies the type of event when an object collides with the plane. */
		PassOptions Type;
		/* Occurs when an object collides with the plane and the pass option Event flag is set. */
		EventBus<const CollisionPlane, const void*> OnPass;

		/* Initializes a new instance of a collision plane. */
		CollisionPlane(_In_ Vector3 normal, _In_ float distance, _In_ PassOptions type)
			: Plane(normal, distance), Type(type), OnPass("CollisionPlane::OnPass")
		{}
	};
}