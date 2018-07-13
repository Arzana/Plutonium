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
	: Type(TextureType::Texture2D), IsDepth(false), Filter(Color::White),
	HorizontalWrap(WrapMode::Repeat), VerticalWrap(WrapMode::Repeat), DepthWrap(WrapMode::Repeat),
	MinFilter(ZoomFilter::Linear), MinFilterMipMap(ZoomFilter::LinearMipMapLinear), MagFilter(ZoomFilter::Linear),
	Gain(0.0f), Range(1.0f), MipMapLevels(4)
{}

bool Plutonium::TextureCreationOptions::operator!=(const TextureCreationOptions & other) const
{
	if (Type == other.Type) return false;
	if (HorizontalWrap == other.HorizontalWrap) return false;
	if (VerticalWrap == other.VerticalWrap) return false;
	if (DepthWrap == other.DepthWrap) return false;
	if (MinFilter == other.MinFilter) return false;
	if (MinFilterMipMap == other.MinFilterMipMap) return false;
	if (MagFilter == other.MagFilter) return false;
	if (Gain == other.Gain) return false;
	if (Range == other.Range) return false;
	if (Filter == other.Filter) return false;
	if (MipMapLevels == other.MipMapLevels) return false;
	return IsDepth != other.IsDepth;
}

bool Plutonium::TextureCreationOptions::operator==(const TextureCreationOptions & other) const
{
	if (Type != other.Type) return false;
	if (HorizontalWrap != other.HorizontalWrap) return false;
	if (VerticalWrap != other.VerticalWrap) return false;
	if (DepthWrap != other.DepthWrap) return false;
	if (MinFilter != other.MinFilter) return false;
	if (MinFilterMipMap != other.MinFilterMipMap) return false;
	if (MagFilter != other.MagFilter) return false;
	if (Gain != other.Gain) return false;
	if (Range != other.Range) return false;
	if (Filter != other.Filter) return false;
	if (MipMapLevels != other.MipMapLevels) return false;
	return IsDepth == other.IsDepth;
}