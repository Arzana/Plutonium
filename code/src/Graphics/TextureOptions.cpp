#include "Graphics\TextureOptions.h"

using namespace Plutonium;

TextureCreationOptions CreateDefaultCubeMap(void)
{
	TextureCreationOptions result;
	result.Type = TextureType::TextureCube;
	result.SetWrapping(WrapMode::ClampToEdge);
	return result;
}

TextureCreationOptions CreateDefaultNoMipMap(void)
{
	TextureCreationOptions result;
	result.MipMapLevels = 0;
	return result;
}

TextureCreationOptions CreateDefaultDepthMap(void)
{
	TextureCreationOptions result;
	result.MipMapLevels = 0;
	result.IsDepth = true;
	result.MagFilter = ZoomFilter::Nearest;
	result.MinFilter = ZoomFilter::Nearest;
	result.MinFilterMipMap = ZoomFilter::Nearest;
	return result;
}

const TextureCreationOptions TextureCreationOptions::Default2D = TextureCreationOptions();
const TextureCreationOptions TextureCreationOptions::DefaultCube = CreateDefaultCubeMap();
const TextureCreationOptions TextureCreationOptions::DefaultNoMipMap = CreateDefaultNoMipMap();
const TextureCreationOptions TextureCreationOptions::DefaultDepthMap = CreateDefaultDepthMap();

const char * Plutonium::_CrtGetVisualTextureType(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
		return "Texture1D";
	case TextureType::Texture2D:
		return "Texture2D";
	case TextureType::Texture3D:
		return "Texture3D";
	case TextureType::TextureRect:
		return "Rectangle Texture";
	case TextureType::TextureCube:
		return "Cubemap Texture";
	case TextureType::TextureBuffer:
		return "Texture Buffer.";
	case TextureType::Texture2DMultiSample:
		return "Multisample Texture2D";
	default:
		return "Unknown";
	}
}

Plutonium::TextureCreationOptions::TextureCreationOptions(void)
	: Type(TextureType::Texture2D), IsDepth(false),
	HorizontalWrap(WrapMode::Repeat), VerticalWrap(WrapMode::Repeat), DepthWrap(WrapMode::Repeat),
	MinFilter(ZoomFilter::Linear), MinFilterMipMap(ZoomFilter::LinearMipMapLinear), MagFilter(ZoomFilter::Linear),
	Gain(0.0f), Range(1.0f), MipMapLevels(4)
{}