#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines a vertex layout that stores a position, normal and texture coordinate. */
	struct Basic3D
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The surface normal of the vertex. */
		Vector3 Normal;
		/* The texture coordinate of the vertex. */
		Vector2 TexCoord;
	};
}