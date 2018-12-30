#pragma once
#include "Graphics/Color.h"

namespace Pu
{
	/* Defines a vertex layout that only stores a position and a color. */
	struct ColoredVertex2D
	{
	public:
		/* The location of the vertex. */
		Vector2 Position;
		/* The color of the vertex. */
		Vector4 Color;

		/* Initializes an empty instance of a colored vertex. */
		ColoredVertex2D(void)
		{}

		/* Initializes a new instance of a colored vertex. */
		ColoredVertex2D(_In_ Vector2 pos, _In_ Vector4 clr)
			: Position(pos), Color(clr)
		{}

		/* Initializes a new instance of a colored vertex. */
		ColoredVertex2D(_In_ Vector2 pos, _In_ Pu::Color clr)
			: Position(pos), Color(clr.ToVector4())
		{}
	};
}