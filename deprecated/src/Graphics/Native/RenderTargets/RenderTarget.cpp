#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Native\RenderTargets\RenderTargetStatus.h"

Plutonium::RenderTarget::RenderTarget(GraphicsAdapter * device, bool attachDepthBuffer, int32 height, int32 width)
	: device(device), drawBufferCnt(0), finalized(false)
{
	Rectangle viewport = device->GetWindow()->GetClientBounds();
	this->width = width > 0 ? width : static_cast<int32>(viewport.GetWidth());
	this->height = height > 0 ? height : static_cast<int32>(viewport.GetHeight());
	glGenFramebuffers(1, &ptrFbo);

	if (attachDepthBuffer) AttachDepthBuffer();
}

Plutonium::RenderTarget::~RenderTarget(void)
{
	glDeleteFramebuffers(1, &ptrFbo);
	glDeleteRenderbuffers(1, &ptrRbo);
	for (size_t i = 0; i < attachments.size(); i++)
	{
		delete_s(attachments.at(i));
	}
}

const Plutonium::RenderTargetAttachment * Plutonium::RenderTarget::Attach(const char * name, AttachmentOutputType type, bool autoBind)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);

	/* Create attachment and push it to the list. */
	bool isDepth = type == AttachmentOutputType::LpDepth || type == AttachmentOutputType::HpDepth;
	RenderTargetAttachment *result = new RenderTargetAttachment(name, type, isDepth ? 0 : drawBufferCnt++, width, height);
	attachments.push_back(result);

	/* Bind the attachment is swapping is not required (most buffers). */
	if (autoBind) result->Bind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return result;
}

void Plutonium::RenderTarget::Finalize(void)
{
	/* Check for invalid render target. */
	ASSERT_IF(finalized, "Attempting to finalize an already finalized render target!");
	ASSERT_IF(attachments.size() < 1, "Attempting to finalize render target with no attachments!");

	finalized = true;
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);

	/* Only add draw buffers if there are any. */
	if (drawBufferCnt > 0)
	{
		/* Create draw buffer list. */
		GLenum *buffers = malloca_s(GLenum, drawBufferCnt);
		for (size_t i = 0, j = 0; i < attachments.size(); i++)
		{
			/* Only add a buffer if it's not the depth attachment. */
			const RenderTargetAttachment *cur = attachments.at(i);
			if (cur->type != AttachmentOutputType::LpDepth && cur->type != AttachmentOutputType::HpDepth) buffers[j++] = cur->attachment;
		}

		/* Push draw list to OpenGL. */
		glDrawBuffers(static_cast<GLsizei>(drawBufferCnt), buffers);
		freea_s(buffers);
	}
	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	/* Make sure the frame buffer is created correctly. */
	RenderTargetStatus status = _CrtInt2Enum<RenderTargetStatus>(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	LOG_THROW_IF(status != RenderTargetStatus::Complete, "Framebuffer could not be finalized (%s)!", _CrtGetRenderTargetStatusStr(status));

	/* Reset the current frame buffer. */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Plutonium::RenderTarget::BlitDepth(void)
{
	/* Copies the depth information from this render target to the default render target. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ptrFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Plutonium::RenderTarget::BindForWriting(const RenderTargetAttachment * attachment)
{
	/* 
	On debug mode, 
	check if the supplied attachment is created using this render target. 
	Also unbind all attached buffers to make sure error checking works.
	*/
#if defined (DEBUG)
	bool notFound = true;
	for (size_t i = 0; i < attachments.size(); i++)
	{
		RenderTargetAttachment *cur = attachments.at(i);
		if (cur == attachment) notFound = false;
		else cur->bound = false;
	}

	LOG_THROW_IF(notFound, "Specified attachment does not belong to this render target!");
#endif

	/* Set the draw buffer to this render target and bind the attachment. */
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);
	attachment->Bind();
}

void Plutonium::RenderTarget::AttachDepthBuffer(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);

	glGenRenderbuffers(1, &ptrRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, ptrRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ptrRbo);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}