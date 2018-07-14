#include "Graphics\Native\RenderTargets\RenderTarget.h"
#include "Graphics\GraphicsAdapter.h"

Plutonium::RenderTarget::RenderTarget(GraphicsAdapter * device)
	: device(device)
{
	Rectangle viewport = device->GetWindow()->GetClientBounds();
	width = static_cast<int32>(viewport.GetWidth());
	height = static_cast<int32>(viewport.GetHeight());
	glGenFramebuffers(1, &ptr);
}

Plutonium::RenderTarget::~RenderTarget(void)
{
	glDeleteFramebuffers(1, &ptr);
	for (size_t i = 0; i < attachments.size(); i++)
	{
		delete_s(attachments.at(i));
	}
}

const Plutonium::RenderTargetAttachment * Plutonium::RenderTarget::Attach(const char * name, AttachmentOutputType type)
{
	device->SetRenderTarget(this);

	RenderTargetAttachment *result = new RenderTargetAttachment(name, type, attachments.size(), width, height);
	attachments.push_back(result);

	device->SetRenderTarget(nullptr);
	return result;
}

void Plutonium::RenderTarget::Finalize(void)
{
	GLenum *buffers = malloca_s(GLenum, attachments.size());
	for (size_t i = 0; i < attachments.size(); i++) buffers[i] = attachments.at(i)->attachment;

	device->SetRenderTarget(this);
	glDrawBuffers(static_cast<GLsizei>(attachments.size()), buffers);
	LOG_THROW_IF(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete!");
	device->SetRenderTarget(nullptr);
}