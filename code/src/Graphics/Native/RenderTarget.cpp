#include "Graphics\Native\RenderTarget.h"
#include "Graphics\GraphicsAdapter.h"

Plutonium::RenderTarget::RenderTarget(GraphicsAdapter * device, RenderTargetType type, int32 width, int32 height)
	: Width(width), Height(height), ptrDb(0)
{
	/* Create frame buffer and depth buffer if needed. */
	GenBuffers(type != RenderTargetType::Color);

	/* Create underlying texture target. */
	const TextureCreationOptions *opt = &(type == RenderTargetType::Depth ? TextureCreationOptions::DefaultDepthMap : TextureCreationOptions::DefaultNoMipMap);
	target = new Texture(width, height, device->GetWindow(), opt, "RenderTarget");

	/* Configure the render target. */
	Configure(type == RenderTargetType::Depth);

	/* Reset the frame buffer to its default state. */
	device->SetRenderTarget(nullptr);
}

Plutonium::RenderTarget::~RenderTarget(void)
{
	delete_s(target);
	glDeleteFramebuffers(1, &ptrFb);
	if (ptrDb) glDeleteRenderbuffers(1, &ptrDb);
}

void Plutonium::RenderTarget::GenBuffers(bool addDepthComponent)
{
	/* Generate frame buffer. */
	glGenFramebuffers(1, &ptrFb);
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFb);

	/* Generate depth buffer is desired. */
	if (addDepthComponent)
	{
		glGenRenderbuffers(1, &ptrDb);
		glBindRenderbuffer(GL_RENDERBUFFER, ptrDb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
	}
}

void Plutonium::RenderTarget::Configure(bool isDepth)
{
	/* Bind texture buffer and depth buffer if needed. */
	if (ptrDb) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ptrDb);
	glFramebufferTexture(GL_FRAMEBUFFER, isDepth ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, target->ptr, 0);

	/* Set the draw buffers if any. */
	if (!isDepth)
	{
		const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);
	}
}