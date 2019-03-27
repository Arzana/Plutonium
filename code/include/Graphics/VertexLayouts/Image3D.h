#pragma once
#include "Core/Math/Vector3.h"

namespace Pu
{
	/* Defines a vertex layout that stores a position and a texture coordinate. */
	struct Image3D
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The texture coordinate of the vertex. */
		Vector2 TexCoord;

		/* Initializes an empty instance of a 3D image vertex. */
		Image3D(void)
		{}

		/* Initializes a new instance of a 3D image vertex. */
		Image3D(_In_ Vector3 pos, _In_ Vector2 uv)
			: Position(pos), TexCoord(uv)
		{}
	};
}