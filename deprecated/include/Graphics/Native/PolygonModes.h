#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the way polygons can be rendered. */
	enum class PolygonModes : GLenum
	{
#if defined(GL_FILL)
		/* The interior of the polygon is filled. */
		Normal = GL_FILL,
#endif
#if defined(GL_POINT)
		/* The vertices defining the polygon will be rendered as points. */
		Point = GL_POINT,
#endif
#if defined(GL_LINE)
		/* The edges defined by the polygon are rendered as lines. */
		Line = GL_LINE,
#endif
	};
}