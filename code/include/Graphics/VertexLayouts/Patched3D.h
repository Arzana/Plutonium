#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines a vertex layout that stores position and two texture coordinates. */
	struct Patched3D
	{
		/* The location of the vertex. */
		Vector3 Position;
		/* The normal of the vertex. */
		Vector3 Normal;
		/* The first texture coordinate of the vertex. */
		Vector2 TexCoord1;
		/* The second texture coordinate of the vertex. */
		Vector2 TexCoord2;
	};
}