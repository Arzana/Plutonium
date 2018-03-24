#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the fragment specific data that can be gathered from the buffer. */
	enum class PixelData : GLenum
	{
#if defined(GL_RED)
		/* The red channel (1 byte). */
		Red = GL_RED,
#endif
#if defined(GL_GREEN)
		/* The green channel (1 byte). */
		Green = GL_GREEN,
#endif
#if defined(GL_BLUE)
		/* The blue channel (1 byte). */
		Blue = GL_BLUE,
#endif
#if defined(GL_RGB)
		/* The red, green and blue channels (3 bytes). */
		RGB = GL_RGB,
#endif
#if defined(GL_RGBA)
		/* All color channels (4 bytes). */
		RGBA = GL_RGBA,
#endif
#if defined(GL_STENCIL_INDEX)
		/* The stencil value (1 byte). */
		Stencil = GL_STENCIL_INDEX
#endif
	};
}