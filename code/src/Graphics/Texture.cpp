#pragma warning(disable:4996)

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Graphics\Texture.h"
#include "Streams\FileReader.h"
#include "Streams\FileUtils.h"
#include "Core\Math\Basics.h"
#include "Core\SafeMemory.h"
#include "Core\Diagnostics\StackTrace.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include <glad\glad.h>
#include <stb\stb_image.h>
#include <stb\stb_image_write.h>

using namespace Plutonium;

Plutonium::Texture::Texture(int32 width, int32 height, int32 mipMaplevels, const char * name)
	: Width(width), Height(height), MipMapLevels(mipMaplevels), name(name ? name : ""),
	frmt(GL_RGBA), ifrmt(GL_RGBA8)
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
}

Texture * Plutonium::Texture::FromFile(const char * path)
{
	FileReader reader(path, true);

	/* Attempt to load texture. */
	int32 w, h, m;
	stbi_set_flip_vertically_on_load(true);
	byte *data = stbi_load(path, &w, &h, &m, 0);

	/* Throw is loading failed. */
	LOG_THROW_IF(!data, "Unable to load texture '%s', reason: %s!", reader.GetFileName(), stbi_failure_reason());

	/* Set texture information. */
	Texture *result = new Texture(w, h, clamp(GetMaxMipMapLevel(w, h), 0, 4));
	result->SetFormat(m);
	result->name = reader.GetFileName();

	/* Load data into texture and return result. */
	result->GenerateTexture(void_ptr(data));
	stbi_image_free(data);

	return result;
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

void Plutonium::Texture::SetData(byte * data)
{
	/* Delete old texture if needed. */
	if (ptr) Dispose();

	/* Generate new texture. */
	GenerateTexture(void_ptr(data));
}

byte * Plutonium::Texture::GetData(void) const
{
	/* On debug mode crash if no texture is specified. */
	ASSERT_IF(!ptr, "Cannot get data from not-loaded texture!");

	/* Initialize buffer result. */
	byte *result = malloc_s(byte, Width * Height * GetChannels());

	/* Request data from GPU. */
	glBindTexture(GL_TEXTURE_2D, ptr);
	glGetTexImage(GL_TEXTURE_2D, 0, frmt, GL_UNSIGNED_BYTE, result);

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

int32 Plutonium::Texture::GetMaxMipMapLevel(int32 w, int32 h)
{
	return 1 + ipart(log2f(static_cast<float>(max(w, h))));
}

void Plutonium::Texture::Dispose(void)
{
	/* Make sure we don't delete an invalid texture. */
	if (ptr)
	{
		glDeleteTextures(1, &ptr);
		_CrtUpdateUsedGPUMemory(-static_cast<int64>(Width * Height));
		ptr = 0;
	}
	else LOG_WAR("Attempting to dispose of not-loaded texture!");
}

void Plutonium::Texture::SetFormat(uint32 channels)
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
		LOG_THROW("Texture doesn't support %d channels!", channels);
		break;
	}
}

void Plutonium::Texture::GenerateTexture(const void * data)
{
	/* Generate storage and bind texture. */
	glGenTextures(1, &ptr);
	glBindTexture(GL_TEXTURE_2D, ptr);

	/* Generate mip map storage and load base texture. */
	glTexStorage2D(GL_TEXTURE_2D, max(1, MipMapLevels), ifrmt, Width, Height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, frmt, GL_UNSIGNED_BYTE, data);
	_CrtUpdateUsedGPUMemory(Width * Height * GetChannels());

	/* Generate desired mip maps. */
	if (MipMapLevels) glGenerateMipmap(GL_TEXTURE_2D);

	/* Set texture use parameters. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MipMapLevels ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

	LOG("Created texture '%s'(%dx%d), %d mipmaps.", name, Width, Height, MipMapLevels);
}