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

Pu::vector<Pu::byte> getDefaultImageLDR(void)
{
	/* Create LDR copy of default image. */
	Pu::vector<Pu::byte> result;
	result.reserve(DEFAULT_IMAGE_COMPONENTS * 4);

	for (Pu::Color cur : DEFAULT_IMAGE)
	{
		result.emplace_back(cur.R);
		result.emplace_back(cur.G);
		result.emplace_back(cur.B);
		result.emplace_back(static_cast<Pu::byte>(255));
	}

	return result;
}

/* STB image sadly only handles ASCII :( */
Pu::ImageInformation Pu::_CrtGetImageInfo(const wstring & path)
{
	int x, y, c;
	const bool result = stbi_info(path.toUTF8().c_str(), &x, &y, &c);

	/* The preferred image component count can override the actual components so if it's set use it otherwise pass the native. */
	return result ? ImageInformation(x, y, PreferredImageComponentCount ? PreferredImageComponentCount : c, stbi_is_hdr(path.toUTF8().c_str())) : ImageInformation();
}

Pu::vector<float> Pu::_CrtLoadImageHDR(const wstring & path)
{
	int x, y, c;
	float *data = stbi_loadf(path.toUTF8().c_str(), &x, &y, &c, PreferredImageComponentCount);
	const wstring name = path.fileName();

	if (data)
	{
		Log::Verbose("Successfully loaded image '%ls'.", name.c_str());
		vector<float> result(data, data + x * y * PreferredImageComponentCount);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%ls', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageHDR();
	}
}

Pu::vector<Pu::byte> Pu::_CrtLoadImageLDR(const wstring & path)
{
	int x, y, c;
	byte *data = stbi_load(path.toUTF8().c_str(), &x, &y, &c, PreferredImageComponentCount);
	const wstring name = path.fileName();

	if (data)
	{
		Log::Verbose("Successfully loaded image '%ls'.", name.c_str());
		vector<byte> result(data, data + x * y * PreferredImageComponentCount);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%ls', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageLDR();
	}
}

Pu::vector<Pu::byte> Pu::_CrtLoadImageLDR(const wstring & path, ImageInformation & info)
{
	int w, h, c;
	byte *data = stbi_load(path.toUTF8().c_str(), &w, &h, &c, PreferredImageComponentCount);
	const wstring name = path.fileName();

	if (data)
	{
		info.Width = static_cast<uint32>(w);
		info.Height = static_cast<uint32>(h);
		info.Components = static_cast<uint32>(PreferredImageComponentCount ? PreferredImageComponentCount : c);
		info.IsHDR = false;

		Log::Verbose("Successfully loaded image '%ls'.", name.c_str());
		vector<byte> result{ data, data + w * h * PreferredImageComponentCount };

		stbi_image_free(data);
		return result;
	}
	else
	{
		info = ImageInformation();
		Log::Error("Unable to load image '%ls', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageLDR();
	}
}

Pu::Format Pu::ImageInformation::GetImageFormat(bool sRGB) const
{
	if (IsHDR)
	{
		switch (Components)
		{ 
		case 1:
			return Format::R32_SFLOAT;
		case 2:
			return Format::R32G32_SFLOAT;
		case 3:
			return Format::R32G32B32_SFLOAT;
		case 4:
			return Format::R32G32B32A32_SFLOAT;
		}
	}
	else
	{
		switch (Components)
		{
		case 1:
			return sRGB ? Format::R8_SRGB : Format::R8_UNORM;
		case 2:
			return sRGB ? Format::R8G8_SRGB : Format::R8G8_UNORM;
		case 3:
			return sRGB ? Format::R8G8B8_SRGB : Format::R8G8B8_UNORM;
		case 4:
			return sRGB ? Format::R8G8B8A8_SRGB : Format::R8G8B8A8_UNORM;
		}
	}

	return Format::Undefined;
}
