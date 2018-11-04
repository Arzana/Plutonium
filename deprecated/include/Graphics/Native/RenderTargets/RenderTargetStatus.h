#include <glad\glad.h>

namespace Plutonium
{
	/* Defines the status that a render target can have after finalizing. */
	enum class RenderTargetStatus
	{
#if defined(GL_FRAMEBUFFER_COMPLETE)
		/* The render target is complete and threw no errors. */
		Complete = GL_FRAMEBUFFER_COMPLETE,
#endif
#if defined(GL_FRAMEBUFFER_UNDEFINED)
		/* The FBO handler is either wrong or it's bound to the default framebuffer whilst it does not excist. */
		Undefined = GL_FRAMEBUFFER_UNDEFINED,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		/* One of the attachments is incomplete. */
		IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		/* The render target defines no useable attachments and is thusly useless. */
		NoAttachments = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		/* GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE whilst one or more draw buffer(s) are specified. */
		FaultyDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
		/* GL_READ_BUFFER and  GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE are GL_NONE whilst one or more read buffer(s) are specified. */
		FaultyReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
#endif
#if defined(GL_FRAMEBUFFER_UNSUPPORTED)
		/* The combination of internal formats is not supported by the graphics driver. */
		NotSupported = GL_FRAMEBUFFER_UNSUPPORTED,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
		/* GL_RENDERBUFFER_SAMPLES or GL_TEXTURE_FIXED_SAMPLE_LOCATIONS are not the same for all attachments. */
		MultisampleMismatch = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
#endif
#if defined(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
		/* One or more framebuffer attachments are layered whilst any populated attachments are not layered. */
		LayerMismatch = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
#endif
	};

	/* Gets a string version of the specified render target status. */
	_Check_return_ inline const char* _CrtGetRenderTargetStatusStr(_In_ RenderTargetStatus status)
	{
		switch (status)
		{
		case Plutonium::RenderTargetStatus::Complete:
			return "Completed";
		case Plutonium::RenderTargetStatus::Undefined:
			return "Undefined framebuffer handle";
		case Plutonium::RenderTargetStatus::IncompleteAttachment:
			return "Incomplete attachment";
		case Plutonium::RenderTargetStatus::NoAttachments:
			return "No useful attachments";
		case Plutonium::RenderTargetStatus::FaultyDrawBuffer:
			return "Draw buffer(s) specified wrongly";
		case Plutonium::RenderTargetStatus::FaultyReadBuffer:
			return "Read buffer(s) specified wrongly";
		case Plutonium::RenderTargetStatus::NotSupported:
			return "Internal format(s) not supported";
		case Plutonium::RenderTargetStatus::MultisampleMismatch:
			return "Attachment multisample mismatch";
		case Plutonium::RenderTargetStatus::LayerMismatch:
			return "Attachment layering mismatch";
		default:
			return "Unknown";
		}
	}
}