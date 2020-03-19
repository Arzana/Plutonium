#pragma once
#include "Core/Math/Vector4.h"

namespace Pu
{
	/* Defines a vertex layout that stores a position, normal, tangent and texture coordinate. */
	struct Advanced3D
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The surface normal of the vertex. */
		Vector3 Normal;
		/* The tangent of the surface normal, W component defines the sign of the bitangent. */
		Vector4 Tangent;
		/* The texture coordinate of the vertex. */
		Vector2 TexCoord;
	};
}