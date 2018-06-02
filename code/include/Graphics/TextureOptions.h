#pragma once
#include <glad\glad.h>
#include <Core\Math\Constants.h>

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

	/* Defines how the texture should me minified or maxified. */
	enum class ZoomFilter : GLint
	{
#if defined(GL_NEAREST)
		/* Uses the nearest (Manhattan) fragment value. */
		Nearest = GL_NEAREST,
#endif
#if defined(GL_LINEAR)
		/* Uses weighted average of vertical and horizontal wrap. */
		Linear = GL_LINEAR,
#endif
#if defined(GL_NEAREST_MIPMAP_NEAREST)
		/* Uses the closest matching mipmap and uses nearest zooming on that (minifying only!). */
		NearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
#endif
#if defined(GL_LINEAR_MIPMAP_NEAREST)
		/* Uses the weighted average mipmap and uses nearest zooming on that (minifying only!). */
		LinearMipMapNearest = GL_LINEAR_MIPMAP_NEAREST,
#endif
#if defined(GL_NEAREST_MIPMAP_LINEAR)
		/* Uses the closest matching mipmap and uses linear zooming on that (minifying only!). */
		NearestMipMapLinear = GL_NEAREST_MIPMAP_LINEAR,
#endif
#if defined(GL_LINEAR_MIPMAP_LINEAR)
		/* Uses the weighted average mipmap and uses linear zooming on that (minifying only!). */
		LinearMipMapLinear = GL_LINEAR_MIPMAP_LINEAR
#endif
	};

	/* Gets a string representing the specified texture type. */
	_Check_return_ const char* _CrtGetVisualTextureType(_In_ TextureType type);

	/* Defines creation parameters for a texture. */
	struct TextureCreationOptions
	{
		/* Defines the default options for a 2D texture. */
		const static TextureCreationOptions Default2D;
		/* Defines the default options for a cubemap texture. */
		const static TextureCreationOptions DefaultCube;
		/* Defines the default options for a 2D texture with no mipmapping. */
		const static TextureCreationOptions DefaultNoMipMap;
		/* Defines the default options for a 2D depth texture. */
		const static TextureCreationOptions DefaultDepthMap;

		/* Defines what kind of texture this is. */
		TextureType Type;
		/* Defines how the horizontal wrapping (S) should work. */
		WrapMode HorizontalWrap;
		/* Defines how the vertical wrapping (T) should work. */
		WrapMode VerticalWrap;
		/* Defines how the depth wrapping (R) should work. */
		WrapMode DepthWrap;
		/* Defines the minifying function of the texture. */
		ZoomFilter MinFilter;
		/* Defines the minifying function of the texture when mipmaps are available. */
		ZoomFilter MinFilterMipMap;
		/* Defines the magnification function of the texture. */
		ZoomFilter MagFilter;
		/* Defines the base brightness of the texture. */
		float Gain;
		/* Defines the color range of the texture. */
		float Range;
		/* Defines the amount of mipmaps levels the texture should generate. */
		int32 MipMapLevels;
		/* Defines whether the texture should store its data as floats instead of bytes. */
		bool IsDepth;

		/* Initializes a new instance of texture options. */
		TextureCreationOptions(void);

		/* Set both horizontal and vertical wrapping to the specified value. */
		inline void SetWrapping(_In_ WrapMode mode)
		{
			HorizontalWrap = mode;
			VerticalWrap = mode;
			DepthWrap = mode;
		}
	};
}