#pragma once
#include "Core/Math/Vector2.h"

namespace Pu
{
	/* Defines a vertex layout that stores a position and a texture coordinate. */
	struct Image2D
	{
	public:
		/* The location of the vertex. */
		Vector2 Position;
		/* The texture coordinate of the vertex. */
		Vector2 TexCoord;

		/* Initializes an empty instance of a 2D image vertex. */
		Image2D(void)
		{}

		/* Initializes a new instance of a 2D image vertex. */
		Image2D(_In_ Vector2 pos, _In_ Vector2 uv)
			: Position(pos), TexCoord(uv)
		{}
	};
}