#include "Graphics\Native\RenderTargets\RenderTargetAttachment.h"
#include "Core\StringFunctions.h"
#include "Core\EnumUtils.h"

GLenum getFormat(Plutonium::AttachmentOutputType type)
{
	switch (type)
	{
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
		return 0;
	}
}

GLenum getType(Plutonium::AttachmentOutputType type)
{
	switch (type)
	{
	case Plutonium::AttachmentOutputType::RGB:
	case Plutonium::AttachmentOutputType::RGBA:
		return GL_UNSIGNED_BYTE;
	case Plutonium::AttachmentOutputType::LpVector3:
	case Plutonium::AttachmentOutputType::LpVector4:
	case Plutonium::AttachmentOutputType::HpVector3:
	case Plutonium::AttachmentOutputType::HpVector4:
		return GL_FLOAT;
	default:
		LOG_THROW("Cannot get type from attachment output type!");
		return 0;
	}
}

Plutonium::RenderTargetAttachment::RenderTargetAttachment(const char * name, AttachmentOutputType type, size_t index, int32 width, int32 height)
	: name(heapstr(name))
{
	/* Check if we can still allow another attachment. */
	LOG_THROW_IF(index > GL_MAX_COLOR_ATTACHMENTS, "Exceeding attachment limit!");

	/* Generate underlying texture with the specified parameters and bind it to the current frame buffer. */
	glGenTextures(1, &ptr);
	glBindTexture(GL_TEXTURE_2D, ptr);
	glTexImage2D(GL_TEXTURE_2D, 0, _CrtEnum2Int(type), static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, getFormat(type), getType(type), nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + index), GL_TEXTURE_2D, ptr, 0);
}

Plutonium::RenderTargetAttachment::~RenderTargetAttachment(void)
{
	free_s(name);
	glDeleteTextures(1, &ptr);
}