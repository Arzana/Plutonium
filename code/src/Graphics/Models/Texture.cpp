#define STB_IMAGE_IMPLEMENTATION

#include "Graphics\Models\Texture.h"
#include "Streams\FileReader.h"
#include "Core\Math\Basics.h"
#include "Core\SafeMemory.h"
#include <glad\glad.h>
#include <stb\stb_image.h>

using namespace Plutonium;

Plutonium::Texture::Texture(int32 width, int32 height, int32 mipMaplevels)
	: Width(width), Height(height), MipMapLevels(mipMaplevels), channels(4), name("")
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
	/* Attempt to load texture with desired 4 mip maps. */
	int32 w, h, m;
	byte *data = stbi_load(path, &w, &h, &m, 4);

	/* Throw is loading failed. */
	LOG_THROW_IF(!data, "Unable to load texture: %s!", path);

	/* Set texture information. */
	Texture *result = new Texture(w, h, max(4, GetMaxMipMapLevel(w, h)));
	result->channels = m;
	result->name = FileReader(path).GetFileName();

	/* Load data into texture and return result. */
	result->GenerateTexture(void_ptr(data));
	return result;
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
#if defined (DEBUG)
	LOG_THROW_IF(!ptr, "Cannot get data from not-loaded texture!");
#endif

	/* Initialize buffer result. */
	byte *result = malloc_s(byte, Width * Height * channels);

	/* Request data from GPU. */
	glBindTexture(GL_TEXTURE_2D, ptr);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, result);

	/* Return supplied data. */
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
		glDeleteTextures(1, &ptr);
		ptr = 0;
	}
	else LOG_WAR("Attempting to dispose of not-loaded texture!");
}

void Plutonium::Texture::GenerateTexture(const void * data)
{
	/* Generate storage and bind texture. */
	glGenTextures(1, &ptr);
	glBindTexture(GL_TEXTURE_2D, ptr);

	/* Generate mip map storage and load base texture. */
	glTexStorage2D(GL_TEXTURE_2D, max(1, MipMapLevels), GL_RGBA8, Width, Height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	/* Generate desired mip maps. */
	if (MipMapLevels) glGenerateMipmap(GL_TEXTURE_2D);

	/* Set texture use parameters. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MipMapLevels ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
}