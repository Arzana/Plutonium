#pragma warning(disable:4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

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
#include <stb\stb_image.h>
#include <stb\stb_image_write.h>

using namespace Plutonium;

constexpr int32 DEFAULT_CHANNEL = 3;

Plutonium::Texture::Texture(int32 width, int32 height, WindowHandler wnd, int32 mipMaplevels, const char * name)
	: wnd(wnd), Width(width), Height(height), MipMapLevels(mipMaplevels), name(name ? heapstr(name) : heapstr("")),
	frmt(GL_RGBA), ifrmt(GL_RGBA8), ptr(0), path(nullptr)
{
	/* On debug mode check if the mipmap level requested is valid. */
#if defined(DEBUG)
	int32 maxLevels = GetMaxMipMapLevel(Width, Height);

	LOG_THROW_IF(Width < 1 || Height < 1, "Width(%d) and height(%d) must be greater than zero!", Width, Height);
	LOG_THROW_IF(mipMaplevels < 0 || MipMapLevels > maxLevels, "Invalid number of mipmaps (%d) requested, Min: %d, Max: %d!", mipMaplevels, 0, maxLevels);
#endif
}

Plutonium::Texture::~Texture(void)
{
	/* If a texture is active delete it. */
	if (ptr) Dispose();
	if (name) free_s(name);
	if (path) free_s(path);
}

int32 Plutonium::Texture::GetChannels(void) const
{
	switch (frmt)
	{
	case(GL_RGB):
		return 3;
	case(GL_RGBA):
		return 4;
	default:
		return -1;
	}
}

void Plutonium::Texture::SetData(byte * data, TextureCreationOptions * config)
{
	/* Delete old texture if needed. */
	if (ptr) Dispose();

	/* Generate new texture. */
	GenerateTexture(&data, config);
}

byte * Plutonium::Texture::GetData(void) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");

	/* Initialize buffer result. */
	byte *result = malloc_s(byte, Width * Height * GetChannels());

	/* Request data from GPU. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindTexture(GL_TEXTURE_2D, ptr);
		glGetTexImage(GL_TEXTURE_2D, 0, frmt, GL_UNSIGNED_BYTE, result);
	}));

	/* Return supplied data. */
	return result;
}

void Plutonium::Texture::SaveAsPng(const char * path)
{
	/* Get texture data. */
	byte *data = GetData();

	/* If the path doesn't fully excist create it. */
	FileReader fr(path, true);
	if (!_CrtDirectoryExists(fr.GetFileDirectory())) _CrtCreateDirectory(fr.GetFileDirectory());

	/* Attempt to save as PNG, no stride for full texture. */
	stbi_flip_vertically_on_write(true);
	int32 result = stbi_write_png(path, Width, Height, GetChannels(), void_ptr(data), 0);
	free_s(data);

	/* On debug throw is saving is not possible. */
	ASSERT_IF(!result, "Unable to save texture(%s) as '%s', reason: %s!", name, path, _CrtGetErrorString());
}

Texture * Plutonium::Texture::FromFile(const char * path, WindowHandler wnd, TextureCreationOptions * config)
{
	FileReader reader(path, true);

	/* Attempt to load texture. */
	int32 w, h, m;
	stbi_set_flip_vertically_on_load(true);
	byte *data = stbi_load(path, &w, &h, &m, 0);

	/* Throw is loading failed. */
	LOG_THROW_IF(!data, "Unable to load texture '%s', reason: %s!", reader.GetFileName(), stbi_failure_reason());

	/* Set texture information. */
	Texture *result = new Texture(w, h, wnd, clamp(GetMaxMipMapLevel(w, h), 0, 4), reader.GetFileNameWithoutExtension());
	if (!result->SetFormat(m)) result->ConvertFormat(&data, m, DEFAULT_CHANNEL);
	result->path = heapstr(path);

	/* Generate OpenGL texture and free stbi data. */
	result->GenerateTexture(&data, config);
	stbi_image_free(data);

	return result;
}

Texture * Plutonium::Texture::FromFile(const char * paths[CUBEMAP_TEXTURE_COUNT], WindowHandler wnd, TextureCreationOptions * config)
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
	Texture *result = new Texture(w[0], h[0], wnd, 0, "Skybox");
	result->path = heapstr(paths[0]);
	for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
	{
		if (!result->SetFormat(m[i])) result->ConvertFormat(&data[i], m[i], DEFAULT_CHANNEL);
	}

	/* Generate OpenGL texture and free stbi data. */
	result->GenerateTexture(data, config);
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
	case(3):
		frmt = GL_RGB;
		ifrmt = GL_RGB8;
		break;
	case(4):
		frmt = GL_RGBA;
		ifrmt = GL_RGBA8;
		break;
	default:
		LOG_WAR("Texture doesn't support %d channels, converting data to RGB!", channels);
		frmt = GL_RGB;
		ifrmt = GL_RGB8;
		return false;
	}

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

void Plutonium::Texture::GenerateTexture(byte ** data, const TextureCreationOptions *config)
{
	/* Set options to default options if needed. */
	if (!config)
	{
		TextureCreationOptions defaultOpt;
		config = &defaultOpt;
	}

	target = config->Type;

	/* Apply texture options associated with the raw data. */
	if (config->Type == TextureType::TextureCube)
	{
		for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
		{
			SetPreDataTransferTextureOptions(data[i], config);
		}
	}
	else SetPreDataTransferTextureOptions(*data, config);

	/* Make sure the OpenGL commands are excecuted on the main thread. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		GLenum target = _CrtEnum2Int(config->Type);

		/* Generate storage and bind texture. */
		glGenTextures(1, &ptr);
		glBindTexture(target, ptr);

		/* Generate mip map storage and load base texture. */
		if (config->Type == TextureType::TextureCube)
		{
			for (size_t i = 0; i < CUBEMAP_TEXTURE_COUNT; i++)
			{
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
				glTexImage2D(target, 0, ifrmt, Width, Height, 0, frmt, GL_UNSIGNED_BYTE, void_ptr(data[i]));
			}
		}
		else
		{
			glTexStorage2D(target, max(1, MipMapLevels), ifrmt, Width, Height);
			glTexSubImage2D(target, 0, 0, 0, Width, Height, frmt, GL_UNSIGNED_BYTE, void_ptr(*data));
		}

		/* Apply texture options associated with rendering */
		SetPostDataTransferTextureOptions(config);
	}));

	_CrtUpdateUsedGPUMemory(Width * Height * GetChannels());

	/* Log creation on debug. */
	LOG("Created texture '%s'(%dx%d), %d mipmaps.", name, Width, Height, MipMapLevels);
}

void Plutonium::Texture::SetPreDataTransferTextureOptions(byte * data, const TextureCreationOptions * config)
{
	/* Check if texture defines brightness gain / scaling. */
	if (config->Gain != 0.0f || config->Range != 1.0f)
	{
		const byte gain = static_cast<byte>(config->Gain);
		const size_t channels = GetChannels(), size = Width * Height * channels;

		/* Apply brightness gain and scale. */
		for (size_t i = 0; i < size; i += channels)
		{
			data[i] = static_cast<byte>((data[i] + gain) * config->Range);
			data[i + 1] = static_cast<byte>((data[i + 1] + gain) * config->Range);
			data[i + 2] = static_cast<byte>((data[i + 2] + gain) * config->Range);
		}
	}
}

void Plutonium::Texture::SetPostDataTransferTextureOptions(const TextureCreationOptions * config)
{
	GLenum target = _CrtEnum2Int(config->Type);
	ASSERT_IF(MipMapLevels && config->Type == TextureType::TextureRect, "Cannot create rectangle texture with mip maps!");

	/* Generate desired mip maps. */
	if (MipMapLevels) glGenerateMipmap(target);

	/* Set texture use parameters. */
	glTexParameteri(target, GL_TEXTURE_WRAP_S, _CrtEnum2Int(config->HorizontalWrap));
	glTexParameteri(target, GL_TEXTURE_WRAP_T, _CrtEnum2Int(config->VerticalWrap));
	if (config->Type == TextureType::Texture3D || config->Type == TextureType::TextureCube) glTexParameteri(target, GL_TEXTURE_WRAP_R, _CrtEnum2Int(config->DepthWrap));
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, MipMapLevels ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
}