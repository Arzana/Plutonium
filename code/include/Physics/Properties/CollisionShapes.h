#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the possible collision shapes for broad and narrow phases. */
	enum class CollisionShapes : uint8
	{
		/* No narrow phase shape, the collider is an AABB. */
		None,
		/* Defines a sphere collider. */
		Sphere,
		/* Defines a capsule collider. */
		Capsule,
		/* Defines an oriented bounding box collider. */
		OBB,
		/* Defines a convex hull collider. */
		Hull,
		/* Defines a mesh collider. */
		Mesh,
		/* Defines a heightmap collider. */
		HeightMap
	};

	/* Converts the collision shape to a human readable version. */
	_Check_return_ inline const char* to_string(_In_ CollisionShapes shape)
	{
		switch (shape)
		{
		case CollisionShapes::None:
			return "AABB";
		case CollisionShapes::Sphere:
			return "Sphere";
		case CollisionShapes::Capsule:
			return "Capsule";
		case CollisionShapes::OBB:
			return "OBB";
		case CollisionShapes::Hull:
			return "Hull";
		case CollisionShapes::Mesh:
			return "Mesh";
		case CollisionShapes::HeightMap:
			return "Heightmap";
		default:
			return "Unknown";
		}
	}
}