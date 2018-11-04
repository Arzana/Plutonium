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
		Vector3 Normal;
		/* The tangent vector at the position. */
		Vector3 Tangent;
		/* The texture UV at the position. */
		Vector2 Texture;
	};
}