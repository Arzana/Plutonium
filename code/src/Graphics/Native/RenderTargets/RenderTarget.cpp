#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "Graphics\GraphicsAdapter.h"
#include "Graphics\Native\RenderTargets\RenderTargetStatus.h"

Plutonium::RenderTarget::RenderTarget(GraphicsAdapter * device, bool attachDepthBuffer)
	: device(device), drawBufferCnt(0), finalized(false)
{
	Rectangle viewport = device->GetWindow()->GetClientBounds();
	width = static_cast<int32>(viewport.GetWidth());
	height = static_cast<int32>(viewport.GetHeight());
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

const Plutonium::RenderTargetAttachment * Plutonium::RenderTarget::Attach(const char * name, AttachmentOutputType type)
{
	/* Check for double depth buffer attachments. */
	if (type == AttachmentOutputType::Depth)
	{
		for (size_t i = 0; i < attachments.size(); i++)
		{
			LOG_THROW_IF(attachments.at(i)->type == AttachmentOutputType::Depth, "Cannot attach two depth buffers to one render target!");
		}
	}

	/* Create attachment and push it to the list. */
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);
	RenderTargetAttachment *result = new RenderTargetAttachment(name, type, type == AttachmentOutputType::Depth ? 0 : drawBufferCnt++, width, height);
	attachments.push_back(result);
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
			if (cur->type != AttachmentOutputType::Depth) buffers[j++] = cur->attachment;
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

void Plutonium::RenderTarget::AttachDepthBuffer(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFbo);

	glGenRenderbuffers(1, &ptrRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, ptrRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ptrRbo);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}