#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

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
	Pu::vector<float> result(DEFAULT_IMAGE_COMPONENTS * 3);
	for (Pu::Color cur : DEFAULT_IMAGE)
	{
		/* Convert from Color to HDR values. */
		const Pu::Vector4 rgba = cur.ToVector4();
		result.emplace_back(rgba.X);
		result.emplace_back(rgba.Y);
		result.emplace_back(rgba.Z);
	}

	return result;
}

Pu::vector<byte> getDefaultImageLDR(void)
{
	/* Create LDR copy of default image. */
	Pu::vector<byte> result(DEFAULT_IMAGE_COMPONENTS * 3);
	for (Pu::Color cur : DEFAULT_IMAGE)
	{
		result.emplace_back(cur.R);
		result.emplace_back(cur.G);
		result.emplace_back(cur.B);
	}

	return result;
}

Pu::ImageInformation Pu::_CrtGetImageInfo(const string & path)
{
	int x, y, c;
	const bool result = stbi_info(path.c_str(), &x, &y, &c);
	return result ? ImageInformation(x, y, c, stbi_is_hdr(path.c_str())) : ImageInformation();
}

Pu::vector<float> Pu::_CrtLoadImageHDR(const string & path)
{
	int x, y, c;
	float *data = stbi_loadf(path.c_str(), &x, &y, &c, 0);
	const string name = _CrtGetFileName(path);

	if (data)
	{
		Log::Verbose("Successfully loaded image '%s'.", name.c_str());
		vector<float> result(data, data + x * y * c);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%s', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageHDR();
	}
}

Pu::vector<byte> Pu::_CrtLoadImageLDR(const string & path)
{
	int x, y, c;
	byte *data = stbi_load(path.c_str(), &x, &y, &c, 0);
	const string name = _CrtGetFileName(path);

	if (data)
	{
		Log::Verbose("Successfully loaded image '%s'.", name.c_str());
		vector<byte> result(data, data + x * y * c);

		stbi_image_free(data);
		return result;
	}
	else
	{
		Log::Error("Unable to load image '%s', reason: '%s'!", name.c_str(), stbi_failure_reason());
		return getDefaultImageLDR();
	}
}