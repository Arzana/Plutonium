#pragma once
#include "Core\Math\Basics.h"
#include "Core\Math\Vector2.h"
#include "Core\Math\Vector3.h"

namespace Plutonium
{
	/* Defines a format for how the vertices are stored in a mesh. */
	struct VertexFormat
	{
		/* The position of the vertx in the mesh. */
		Vector3 Position;
		/* The face normal at the position. */
		Vector2 Normal;
		/* The texture UV at the position. */
		Vector2 Texture;

		/* Calculates the 3D version of the normal. */
		_Check_return_ inline Vector3 Get3DNormal(void) const
		{
			float z = sqrtf(1.0f - sqr(Normal.X) - sqr(Normal.Y));
			return Vector3(Normal.X, Normal.Y, z);
		}
	};
}