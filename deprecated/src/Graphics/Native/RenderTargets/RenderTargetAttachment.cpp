#pragma warning(disable:4996)
#include "Graphics\Native\RenderTargets\RenderTargetAttachment.h"
#include "Graphics\Diagnostics\FrameInfo.h"
#include "Core\StringFunctions.h"
#include "Core\EnumUtils.h"
#include "Streams\FileReader.h"
#include "Streams\FileUtils.h"
#include "Core\Diagnostics\StackTrace.h"
#include <stb\stb_image_write.h>

/* Default case will halt excecution and will therefor never need to return. */
#pragma warning (push)
#pragma warning (disable:4715)
GLenum getFormat(Plutonium::AttachmentOutputType type)
{
	switch (type)
	{
	case Plutonium::AttachmentOutputType::LpDepth:
	case Plutonium::AttachmentOutputType::HpDepth:
		return GL_DEPTH_COMPONENT;
	case Plutonium::AttachmentOutputType::RGB:
	case Plutonium::AttachmentOutputType::LpVector3:
	case Plutonium::AttachmentOutputType::HpVector3:
		return GL_RGB;
	case Plutonium::AttachmentOutputType::RGBA:
	case Plutonium::AttachmentOutputType::LpVector4:
	case Plutonium::AttachmentOutputType::HpVector4:
		return GL_RGBA;
	default:
		LOG_THROW("Cannot get format from attachment output type!");
	}
}
#pragma warning (pop)

/* Default case will halt excecution and will therefor never need to return. */
#pragma warning (push)
#pragma warning (disable:4715)
GLenum getType(Plutonium::AttachmentOutputType type)
{
	switch (type)
	{
	case Plutonium::AttachmentOutputType::RGB:
	case Plutonium::AttachmentOutputType::RGBA:
		return GL_UNSIGNED_BYTE;
	case Plutonium::AttachmentOutputType::LpDepth:
	case Plutonium::AttachmentOutputType::HpDepth:
	case Plutonium::AttachmentOutputType::LpVector3:
	case Plutonium::AttachmentOutputType::LpVector4:
	case Plutonium::AttachmentOutputType::HpVector3:
	case Plutonium::AttachmentOutputType::HpVector4:
		return GL_FLOAT;
	default:
		LOG_THROW("Cannot get type from attachment output type!");
	}
}
#pragma warning (pop)

size_t getChannels(Plutonium::AttachmentOutputType type)
{
	switch (type)
	{
	case Plutonium::AttachmentOutputType::LpDepth:
	case Plutonium::AttachmentOutputType::HpDepth:
		return 1;
	case Plutonium::AttachmentOutputType::RGB:
	case Plutonium::AttachmentOutputType::LpVector3:
	case Plutonium::AttachmentOutputType::HpVector3:
		return 3;
	case Plutonium::AttachmentOutputType::RGBA:
	case Plutonium::AttachmentOutputType::LpVector4:
	case Plutonium::AttachmentOutputType::HpVector4:
		return 4;
	default:
		return  0;
	}
}

void Plutonium::RenderTargetAttachment::SaveAsPng(const char * path, bool flipVertically) const
{
	/* Gets the current channels and size defined by the texture. */
	size_t channels = getChannels(type);
	size_t size = width * height * channels;
	GLenum underlying = getType(type);

	/* Throw if the type is any vector. */
	LOG_THROW_IF(underlying == GL_FLOAT && channels > 1, "Cannot convert vector types to png image at this point!");
	glBindTexture(GL_TEXTURE_2D, ptr);

	/* Create and populate color buffer, floats will always bind to RGBA and bytes will bind to whatever channel is specified by the texture. */
	byte *data = malloca_s(byte, width * height * (underlying == GL_FLOAT ? 4 : channels));
	if (underlying == GL_FLOAT)
	{
		/* Get the raw float data. */
		float *raw = malloca_s(float, size);
		glGetTexImage(GL_TEXTURE_2D, 0, getFormat(type), underlying, raw);

		/* Convert data to RGBA greyscale and hard set the channels to 4. */
		data = _CrtToGreyscale(raw, size, false, true);
		channels = 4;

		/* Free temporary buffer. */
		freea_s(raw);
	}
	else glGetTexImage(GL_TEXTURE_2D, 0, getFormat(type), underlying, data);

	/* Create directory if needed. */
	FileReader fr(path, true);
	if (!_CrtDirectoryExists(fr.GetFileDirectory())) _CrtCreateDirectory(fr.GetFileDirectory());

	/* Try to save the underlying texture. */
	stbi_flip_vertically_on_write(flipVertically);
	int32 result = stbi_write_png(path, width, height, static_cast<int32>(channels), data, 0);

	/* Free buffer and assert if an error occured. */
	freea_s(data);
	if (result)
	{
		LOG("Saved render target attachment '%s' as '%s'.", name, path);
	}
	else
	{
		ASSERT("Unable to save render target attachment '%s' as '%s', reason: '%s'!", name, path, _CrtGetErrorString());
	}
}

Plutonium::RenderTargetAttachment::RenderTargetAttachment(const char * name, AttachmentOutputType type, size_t index, int32 width, int32 height)
	: name(heapstr(name)), type(type), width(width), height(height)
#if defined (DEBUG)
	, bound(false)
#endif
{
	/* Set the border and the attachment type. */
	constexpr GLint border[] = { 1, 1, 1, 1 };
	bool isDepth = type == AttachmentOutputType::LpDepth || type == AttachmentOutputType::HpDepth;
	attachment = isDepth ? GL_DEPTH_ATTACHMENT : static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index);

	/* Check if we can still allow another attachment. */
	LOG_THROW_IF(index > GL_MAX_COLOR_ATTACHMENTS, "Exceeding attachment limit!");

	/* Generate underlying texture with the specified parameters. */
	glGenTextures(1, &ptr);
	glBindTexture(GL_TEXTURE_2D, ptr);
	glTexImage2D(GL_TEXTURE_2D, 0, _CrtEnum2Int(type), static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, getFormat(type), getType(type), nullptr);

	/* Set the parameters for the underlying sampler. */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isDepth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, isDepth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
}

void Plutonium::RenderTargetAttachment::Bind(void) const
{
#if defined (DEBUG)
	bound = true;
#endif
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, ptr, 0);
}

Plutonium::RenderTargetAttachment::~RenderTargetAttachment(void)
{
	free_s(name);
	glDeleteTextures(1, &ptr);
}