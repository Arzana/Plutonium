#pragma warning(disable:4996)
#include "Graphics\Texture.h"
#include "Streams\FileReader.h"
#include "Streams\FileUtils.h"
#include "Core\Math\Basics.h"
#include "Core\SafeMemory.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Core\EnumUtils.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Graphics\Native\Window.h"
#include "Core\StringFunctions.h"
#include <glad\glad.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#if !defined (DEBUG)
#define STBI_FAILURE_USERMSG
#else
#define STBIW_ASSERT(x)	ASSERT_IF(!(x), "STB image writer raised an error!")
#endif

#include <stb\stb_image.h>
#include <stb\stb_image_write.h>

using namespace Plutonium;

constexpr int32 DEFAULT_CHANNEL = 3;

Plutonium::Texture::Texture(int32 width, int32 height, WindowHandler wnd, const TextureCreationOptions * config, const char * name)
	: Width(width), Height(height), MipMapLevels(config->MipMapLevels), name(name ? heapstr(name) : heapstr("")),
	ptr(0), path(nullptr), wnd(wnd), config(*config)
{
	/* On debug mode check if the mipmap level requested is valid. */
#if defined(DEBUG)
	int32 maxLevels = GetMaxMipMapLevel(Width, Height);

	LOG_THROW_IF(Width < 1 || Height < 1, "Width(%d) and height(%d) must be greater than zero!", Width, Height);
	LOG_THROW_IF(MipMapLevels < 0 || MipMapLevels > maxLevels, "Invalid number of mipmaps (%d) requested, Min: %d, Max: %d!", MipMapLevels, 0, maxLevels);
#endif

	/* Set the default format to depth or RGBA depending on supplied options. */
	SetFormat(config->IsDepth ? 1 : 4);
}

Plutonium::Texture::~Texture(void)
{
	/* If a texture is active delete it. */
	if (ptr) Dispose();
	if (name) free_s(name);
	if (path) free_s(path);
}

bool Plutonium::Texture::operator!=(const Texture & other) const
{
	if (eqlstr(name, other.name)) return false;
	if (eqlstr(path, other.path)) return false;
	return config != other.config;
}

int32 Plutonium::Texture::GetChannels(void) const
{
	switch (frmt)
	{
	case (GL_RED):
	case (GL_DEPTH_COMPONENT):
		return 1;
	case (GL_RGB):
		return 3;
	case (GL_RGBA):
		return 4;
	default:
		return -1;
	}
}

void Plutonium::Texture::SetData(byte * data)
{
	/* Delete old texture if needed. */
	if (ptr) Dispose();

	/* Generate new texture. */
	GenerateTexture(&data);
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::Texture::SaveAsPng(const char * path, bool flipVertically) const
{
	/* Get texture data. */
	ASSERT_IF(config.IsDepth, "Cannot save depth maps to as png!");
	byte *data = malloca_s(byte, Width * Height * GetChannels());
	GetData(data);

	/* If the path doesn't fully excist create it. */
	FileReader fr(path, true);
	if (!_CrtDirectoryExists(fr.GetFileDirectory())) _CrtCreateDirectory(fr.GetFileDirectory());

	/* Attempt to save as PNG, no stride for full texture. */
	stbi_flip_vertically_on_write(flipVertically);
	int32 result = stbi_write_png(path, Width, Height, GetChannels(), void_ptr(data), 0);
	freea_s(data);

	/* On debug throw is saving is not possible. */
	LOG_WAR_IF(!result, "Unable to save texture(%s) as '%s', reason: %s!", name, path, _CrtGetErrorString());
}
#pragma warning (pop)

Texture * Plutonium::Texture::FromFile(const char * path, WindowHandler wnd, const TextureCreationOptions * config)
{
	FileReader reader(path, true);

	/* Attempt to load texture. */
	int32 w, h, m;
	stbi_set_flip_vertically_on_load(true);
	byte *data = stbi_load(path, &w, &h, &m, 0);

	/* Throw is loading failed or log warnings. */
	LOG_THROW_IF(!data, "Unable to load texture '%s', reason: %s!", reader.GetFileName(), stbi_failure_reason());

	/* Set texture information. */
	Texture *result = new Texture(w, h, wnd, config, reader.GetFileNameWithoutExtension());
	if (!result->SetFormat(m)) result->ConvertFormat(&data, m, DEFAULT_CHANNEL);
	result->path = heapstr(path);

	/* Generate OpenGL texture and free stbi data. */
	result->GenerateTexture(&data);
	stbi_image_free(data);

	return result;
}

Texture * Plutonium::Texture::FromFile(const char * paths[CUBEMAP_TEXTURE_COUNT], WindowHandler wnd, const TextureCreationOptions * config)
{
	/* Setup buffers for cube map textures. */
	byte *data[CUBEMAP_TEXTURE_COUNT];
	int32 w[CUBEMAP_TEXTURE_COUNT], h[CUBEMAP_TEXTURE_COUNT], m[CUBEMAP_TEXTURE_COUNT];
	stbi_set_flip_vertically_on_load(false);

	/* Load individual textures. */
	for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
	{
		FileReader reader(paths[i], true);
		data[i] = stbi_load(paths[i], &w[i], &h[i], &m[i], 0);

		/* Check for errors. */
		LOG_THROW_IF(!data[i], "Unable to load texture '%s', reason: %s!", reader.GetFileName(), stbi_failure_reason());
		ASSERT_IF(i > 0 ? (w[i] != w[i - 1] || h[i] != h[i - 1]) : false, "Cannot use specified texture for cubemap (texture sizes differ)!");
	}

	/* Set texture information. */
	Texture *result = new Texture(w[0], h[0], wnd, config, "Skybox");
	result->path = heapstr(paths[0]);
	for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
	{
		if (!result->SetFormat(m[i])) result->ConvertFormat(&data[i], m[i], DEFAULT_CHANNEL);
	}

	/* Generate OpenGL texture and free stbi data. */
	result->GenerateTexture(data);
	for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++) stbi_image_free(data[i]);

	return result;
}

int32 Plutonium::Texture::GetMaxMipMapLevel(int32 w, int32 h)
{
	return 1 + ipart(log2f(static_cast<float>(max(w, h))));
}

void Plutonium::Texture::Dispose(void)
{
	/* Make sure we don't delete an invalid texture. */
	if (ptr)
	{
		wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
		{
			glDeleteTextures(1, &ptr);
		}));

		_CrtUpdateUsedGPUMemory(-static_cast<int64>(Width * Height));
		ptr = 0;
	}
	else LOG_WAR("Attempting to dispose of not-loaded texture!");
}

bool Plutonium::Texture::SetFormat(uint32 channels)
{
	switch (channels)
	{
	case(1):
		if (config.IsDepth)
		{
			frmt = GL_DEPTH_COMPONENT;
			ifrmt = GL_DEPTH_COMPONENT;
		}
		else
		{
			frmt = GL_RED;
			ifrmt = GL_R8;
		}
		break;
	case(3):
		frmt = GL_RGB;
		ifrmt = GL_RGB8;
		break;
	case(4):
		frmt = GL_RGBA;
		ifrmt = GL_RGBA8;
		break;
	default:
		LOG_WAR("Texture doesn't support %d channel(s), converting data to RGB!", channels);
		frmt = GL_RGB;
		ifrmt = GL_RGB8;
		type = GL_UNSIGNED_BYTE;
		return false;
	}

	type = config.IsDepth ? GL_FLOAT : GL_UNSIGNED_BYTE;
	return true;
}

void Plutonium::Texture::ConvertFormat(byte ** data, int32 srcChannels, int32 destChannels)
{
	/* Allocate new buffer. */
	byte *result = malloc_s(byte, Width * Height * destChannels);
	size_t size = Width * Height * srcChannels;

	/* Copy over raw data. */
	for (size_t i = 0, j = 0; i < size; i += srcChannels)
	{
		for (size_t k = 0; k < destChannels; k++, j++)
		{
			result[j] = (*data)[i];
		}
	}

	/* Release old buffer and set new buffer. */
	free_s(*data);
	*data = result;
}

void Plutonium::Texture::GenerateTexture(byte ** data)
{
	/* Apply texture options associated with the raw data. */
	if (!config.IsDepth)
	{
		if (config.Type == TextureType::TextureCube)
		{
			for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
			{
				SetPreDataTransferTextureOptions(data[i]);
			}
		}
		else SetPreDataTransferTextureOptions(*data);
	}

	/* Make sure the OpenGL commands are excecuted on the main thread. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		GLenum target = _CrtEnum2Int(config.Type);

		/* Generate storage and bind texture. */
		glGenTextures(1, &ptr);
		glBindTexture(target, ptr);

		/* Generate mip map storage and load base texture. */
		if (config.Type == TextureType::TextureCube)
		{
			for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
			{
				target = static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
				glTexImage2D(target, 0, ifrmt, Width, Height, 0, frmt, type, void_ptr(data[i]));
			}
		}
		else
		{
			glTexStorage2D(target, max(1, MipMapLevels), ifrmt, Width, Height);
			glTexSubImage2D(target, 0, 0, 0, Width, Height, frmt, type, void_ptr(*data));
		}

		/* Apply texture options associated with rendering */
		SetPostDataTransferTextureOptions();
	}));

	_CrtUpdateUsedGPUMemory(Width * Height * GetChannels());

	/* Log creation on debug. */
	LOG("Created texture '%s'(%dx%d), %d mipmaps.", name, Width, Height, MipMapLevels);
}

void Plutonium::Texture::SetPreDataTransferTextureOptions(byte * data)
{
	const size_t channels = GetChannels(), size = Width * Height * channels;

	/* Check if texture defines brightness gain / scaling. */
	if (config.Gain != 0.0f || config.Range != 1.0f || config.Filter != Color::White())
	{
		const byte gain = static_cast<byte>(config.Gain);
		const Vector4 filter = config.Filter.ToVector4();

		/* Apply brightness gain and scale. Apply color filter as well. */
		for (size_t i = 0; i < size; i += channels)
		{
			data[i] = static_cast<byte>((data[i] + gain) * config.Range * filter.X);
			if (channels > 1)
			{
				data[i + 1] = static_cast<byte>((data[i + 1] + gain) * config.Range * filter.Y);
				data[i + 2] = static_cast<byte>((data[i + 2] + gain) * config.Range * filter.Z);
				if (channels > 3) data[i + 3] = static_cast<byte>(data[i + 3] * filter.W);
			}
		}
	}
}

void Plutonium::Texture::SetPostDataTransferTextureOptions(void)
{
	GLenum target = _CrtEnum2Int(config.Type);
	ASSERT_IF(MipMapLevels && config.Type == TextureType::TextureRect, "Cannot create rectangle texture with mip maps!");

	/* Generate desired mip maps. */
	if (MipMapLevels) glGenerateMipmap(target);

	/* Set texture use parameters. */
	glTexParameteri(target, GL_TEXTURE_WRAP_S, _CrtEnum2Int(config.HorizontalWrap));
	glTexParameteri(target, GL_TEXTURE_WRAP_T, _CrtEnum2Int(config.VerticalWrap));
	if (config.Type == TextureType::Texture3D || config.Type == TextureType::TextureCube) glTexParameteri(target, GL_TEXTURE_WRAP_R, _CrtEnum2Int(config.DepthWrap));
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, _CrtEnum2Int(config.MagFilter));
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, MipMapLevels ? _CrtEnum2Int(config.MinFilterMipMap) : _CrtEnum2Int(config.MinFilter));
}

void Plutonium::Texture::GetDataAsBytes(byte * buffer) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");
	ASSERT_IF(config.Type == TextureType::TextureCube, "Cannot get data from cube map at this point!");

	/* Populate buffer. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindTexture(GL_TEXTURE_2D, ptr);
		glGetTexImage(GL_TEXTURE_2D, 0, frmt, GL_UNSIGNED_BYTE, buffer);
	}));
}

void Plutonium::Texture::GetDataAsFloats(float * buffer) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");
	ASSERT_IF(config.Type == TextureType::TextureCube, "Cannot get data from cube map at this point!");

	/* Populate buffer. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindTexture(GL_TEXTURE_2D, ptr);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);
	}));
}

void Plutonium::Texture::GetDataAsRGB(Color * buffer) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");
	ASSERT_IF(config.Type == TextureType::TextureCube, "Cannot get data from cube map at this point!");

	/* Get raw data. */
	byte *raw = malloca_s(byte, Width * Height * 3);
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindTexture(GL_TEXTURE_2D, ptr);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, raw);
	}));

	/* Convert raw data to RGB (cannot just cast as the size of the color struct is build for RGBA). */
	for (size_t i = 0, j = 0; i < Width * Height; i += 3, j++)
	{
		buffer[j] = Color(raw[i], raw[i + 1], raw[i + 2]);
	}

	/* Release temporary buffer. */
	freea_s(raw);
}

void Plutonium::Texture::GetDataAsRGBA(Color * buffer) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");
	ASSERT_IF(config.Type == TextureType::TextureCube, "Cannot get data from cube map at this point!");

	/* Get raw data. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindTexture(GL_TEXTURE_2D, ptr);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}));
}