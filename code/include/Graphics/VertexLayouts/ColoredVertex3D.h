#pragma once
#include "Graphics/Color.h"

namespace Pu
{
	/* Defines a vertex layout that only stores a position and a color. */
	struct ColoredVertex3D
	{
	public:
		/* The location of the vertex. */
		Vector3 Position;
		/* The color of the vertex. */
		Vector4 Color;

		/* Initializes an empty instance of a colored vertex. */
		ColoredVertex3D(void)
		{}

		/* Initializes a new instance of a colored vertex. */
		ColoredVertex3D(_In_ Vector3 pos, _In_ Vector4 clr)
			: Position(pos), Color(clr)
		{}

		/* Initializes a new instance of a colored vertex. */
		ColoredVertex3D(_In_ Vector3 pos, _In_ Pu::Color clr)
			: Position(pos), Color(clr.ToVector4())
		{}
	};
}