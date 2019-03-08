#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef _WIN32
#define STBI_WINDOWS_UTF8
#endif

#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#else
#define STBI_ASSERT(x) if (!(x)) { Pu::Log::Fatal("STB image writer raised an error!"); }
#endif

#include "Graphics/Resources/ImageHandler.h"
#include "Core/Diagnostics/Logging.h"
#include "Streams/FileUtils.h"
#include "Graphics/Color.h"
#include <stb/stb/stb_image.h>

/* Defines the default image LDR (lime and magenta checkerboard) */
static const Pu::Color DEFAULT_IMAGE[] =
{
	Pu::Color::Lime(),
	Pu::Color::Magenta(),
	Pu::Color::Lime(),

	Pu::Color::Magenta(),
	Pu::Color::Lime(),
	Pu::Color::Magenta(),

	Pu::Color::Lime(),
	Pu::Color::Magenta(),
	Pu::Color::Lime(),
};

/* Defines the amount of colors in the default image. */
constexpr size_t DEFAULT_IMAGE_COMPONENTS = sizeof(DEFAULT_IMAGE) / sizeof(Pu::Color);

Pu::vector<float> getDefaultImageHDR(void)
{
	/* Create HDR copy of default image. */
	Pu::vector<float> result;
	result.reserve(DEFAULT_IMAGE_COMPONENTS * 4);

	for (Pu::Color cur : DEFAULT_IMAGE)
	{
		/* Convert from Color to HDR values. */
		const Pu::Vector4 rgba = cur.ToVector4();
		result.emplace_back(rgba.X);
		result.emplace_back(rgba.Y);
		result.emplace_back(rgba.Z);
		result.emplace_back(1.0f);
	}

	return result;
}

Pu::vector<byte> getDefaultImageLDR(void)
{
	/* Create LDR copy of default image. */
	Pu::vector<byte> result;
	result.reserve(DEFAULT_IMAGE_COMPONENTS * 4);

	for (Pu::Color cur : DEFAULT_IMAGE)
	{
		result.emplace_back(cur.R);
		result.emplace_back(cur.G);
		result.emplace_back(cur.B);
		result.emplace_back(static_cast<byte>(255));
	}

	return result;
}

/* STB image sadly only handles ASCII :( */
Pu::ImageInformation Pu::_CrtGetImageInfo(const wstring & path)
{
	int x, y, c;
	const bool result = stbi_info(path.toUTF8().c_str(), &x, &y, &c);
	return result ? ImageInformation(x, y, c, stbi_is_hdr(path.toUTF8().c_str())) : ImageInformation();
}

Pu::vector<float> Pu::_CrtLoadImageHDR(const wstring & path)
{
	int x, y, c;
	float *data = stbi_loadf(path.toUTF8().c_str(), &x, &y, &c, 0);
	const wstring name = _CrtGetFileName(path);

	if (data)
	{
		Log::Verbose("Successfully loaded image '%ls'.", name.c_str());
		vector<float> result(data, data + x * y * c);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%ls', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageHDR();
	}
}

Pu::vector<byte> Pu::_CrtLoadImageLDR(const wstring & path)
{
	int x, y, c;
	byte *data = stbi_load(path.toUTF8().c_str(), &x, &y, &c, 0);
	const wstring name = _CrtGetFileName(path);

	if (data)
	{
		Log::Verbose("Successfully loaded image '%ls'.", name.c_str());
		vector<byte> result(data, data + x * y * c);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%ls', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageLDR();
	}
}

Pu::Format Pu::ImageInformation::GetImageFormat(void) const
{
	switch (Components)
	{
	case (1):
		return IsHDR ? Format::R32_SFLOAT : Format::R8_SRGB;
	case (2):
		return IsHDR ? Format::R32G32_SFLOAT : Format::R8G8_SRGB;
	case (3):
		return IsHDR ? Format::R32G32B32_SFLOAT : Format::R8G8B8_SRGB;
	case (4):
		return IsHDR ? Format::R32G32B32A32_SFLOAT : Format::R8G8B8A8_SRGB;
	default:
		return Format::Undefined;
	}
}
