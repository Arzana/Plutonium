#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifndef _DEBUG
#define STBI_FAILURE_USERMSG
#else
#define STBIW_ASSERT(x) if (!(x)) { Pu::Log::Fatal("STB image writer raised an error!"); }
#endif

#include "Graphics/Resources/ImageHandler.h"
#include "Core/Diagnostics/Logging.h"
#include "Streams/FileUtils.h"
#include "Graphics/Color.h"
#include <stb/stb/stb_image.h>

Pu::vector<float> Pu::_CrtLoadImage(const string & path, ImageInformation & info, int32 desiredChannels)
{
	static const Pu::Vector4 DEFAULT_IMAGE_DATA[] =
	{
		Pu::Color::Lime().ToVector4(),
		Pu::Color::Magenta().ToVector4(),
		Pu::Color::Lime().ToVector4(),

		Pu::Color::Magenta().ToVector4(),
		Pu::Color::Lime().ToVector4(),
		Pu::Color::Magenta().ToVector4(),

		Pu::Color::Lime().ToVector4(),
		Pu::Color::Magenta().ToVector4(),
		Pu::Color::Lime().ToVector4(),
	};

	int x, y, c;
	const float *data = stbi_loadf(path.c_str(), &x, &y, &c, desiredChannels);

	const string name = _CrtGetFileName(path);

	if (data)
	{
		Log::Verbose("Successfully loaded image '%s'.", name.c_str());

		info.Width = static_cast<uint32>(x);
		info.Height = static_cast<uint32>(y);
		info.Components = static_cast<uint32>(c);

		return vector<float>(data, data + x * y * c);
	}
	else
	{
		Log::Error("Unable to load image '%s', reason: '%s'!", name.c_str(), stbi_failure_reason());

		info.Width = 3;
		info.Height = 3;
		info.Components = 4;

		return vector<float>(reinterpret_cast<const float*>(DEFAULT_IMAGE_DATA), reinterpret_cast<const float*>(DEFAULT_IMAGE_DATA + sizeof(DEFAULT_IMAGE_DATA) / sizeof(Vector4)));
	}
}

Pu::ImageLoadTask::ImageLoadTask(const string & path, int32 desiredChannels)
	: path(path), channels(desiredChannels)
{}

Pu::Task::Result Pu::ImageLoadTask::Execute(void)
{
	result = _CrtLoadImage(path, info, channels);
	return Result();
}