#pragma once

namespace Pu
{
	/* Defines the possible collision shapes for broad and narrow phases. */
	enum class CollisionShapes
	{
		/* Defines a sphere collider (usable for broad phase). */
		Sphere,
		/* Defines an axis-aligned bounding box collider (usable for broad phase). */
		AABB,
		/* Defines a capsule collider. */
		Capsule,
		/* Defines an oriented bounding box collider. */
		OBB,
		/* Defines a convex hull collider. */
		Hull,
		/* Defines a mesh collider. */
		Mesh
	};
}