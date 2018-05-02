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

	/* Defines how the texture should be bound in OpenGL. */
	enum class TextureType : GLenum
	{
#if defined(GL_TEXTURE_1D)
		/* A one dimensional texture. */
		Texture1D = GL_TEXTURE_1D,
#endif
#if defined(GL_TEXTURE_2D)
		/* A two dimensional texture. */
		Texture2D = GL_TEXTURE_2D,
#endif
#if defined(GL_TEXTURE_3D)
		/* A three dimensional texture. */
		Texture3D = GL_TEXTURE_3D,
#endif
#if defined(GL_TEXTURE_RECTANGLE)
		/* A two dimensional texture with no mipmaps. */
		TextureRect = GL_TEXTURE_RECTANGLE,
#endif
#if defined(GL_TEXTURE_CUBE_MAP)
		/* A texture that consists of six two dimensional textures. */
		TextureCube = GL_TEXTURE_CUBE_MAP,
#endif
#if defined(GL_TEXTURE_BUFFER)
		/* A one dimensional texture that uses a buffer as storage. */
		TextureBuffer = GL_TEXTURE_BUFFER,
#endif
#if defined(GL_TEXTURE_2D_MULTISAMPLE)
		/* A texture that can have multiple samples per pixel. */
		Texture2DMultiSample = GL_TEXTURE_2D_MULTISAMPLE
#endif
	};

	/* Defines creation parameters for a texture. */
	struct TextureCreationOptions
	{
		/* Defines what kind of texture this is. */
		TextureType Type;
		/* Defines how the horizontal wrapping (S) should work. */
		WrapMode HorizontalWrap;
		/* Defines how the vertical wrapping (T) should work. */
		WrapMode VerticalWrap;
		/* Defines how the depth wrapping (R) should work. */
		WrapMode DepthWrap;
		/* Defines the base brightness of the texture. */
		float Gain;
		/* Defines the color range of the texture. */
		float Range;

		/* Initializes a new instance of texture options. */
		TextureCreationOptions(void)
			: Type(TextureType::Texture2D),
			HorizontalWrap(WrapMode::Repeat), VerticalWrap(WrapMode::Repeat), DepthWrap(WrapMode::Repeat),
			Gain(0.0f), Range(1.0f)
		{}

		/* Set both horizontal and vertical wrapping to the specified value. */
		inline void SetWrapping(_In_ WrapMode mode)
		{
			HorizontalWrap = mode;
			VerticalWrap = mode;
		}
	};
}