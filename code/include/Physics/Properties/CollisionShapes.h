#pragma once

namespace Pu
{
	/* Defines the possible collision shapes for broad and narrow phases. */
	enum class CollisionShapes
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
		Mesh
	};
}