#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Defines how texture coordinates should be wrapped. */
	enum class WrapMode : GLint
	{
#if defined(GL_CLAMP_TO_EDGE)
		/* Texture coordinates outside of the texture will be replaced with the closest edge fragment. */
		ClampToEdge = GL_CLAMP_TO_EDGE,
#endif
#if defined(GL_CLAMP_TO_BORDER)
		/* Texture coordinates outside of the texture will be replaced with the specified border color. */
		ClampToBorder = GL_CLAMP_TO_BORDER,
#endif
#if defined(GL_REPEAT)
		/* Texture coordinates will not be clamped but will repeat. */
		Repeat = GL_REPEAT,
#endif
#if defined(GL_MIRRORED_REPEAT)
		/* Texture coordinates will not be clamped but will repeat in alternating order. */
		MirroredRepeat = GL_MIRRORED_REPEAT,
#endif
#if defined(GL_MIRROR_CLAMP_TO_EDGE)
		/* Texture coordinates outside the texture will be replaces with the closest mirrored edge fragment. */
		MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
#endif
	};

	/* Defines creation parameters for a texture. */
	struct TextureCreationOptions
	{
		/* Defines how the horizontal wrapping (S) should work. */
		WrapMode HorizontalWrap;
		/* Defines how the vertical wrapping (T) should work. */
		WrapMode VerticalWrap;

		/* Initializes a new instance of texture options. */
		TextureCreationOptions(void)
			: HorizontalWrap(WrapMode::Repeat), VerticalWrap(WrapMode::Repeat)
		{}
	};
}