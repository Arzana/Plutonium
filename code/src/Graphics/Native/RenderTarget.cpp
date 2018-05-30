#include "Graphics\Native\RenderTarget.h"
#include "Graphics\GraphicsAdapter.h"
#include <glad\glad.h>

Plutonium::RenderTarget::RenderTarget(GraphicsAdapter * device, int32 width, int32 height)
	: Width(width), Height(height)
{
	/* Generate frame and depth buffers. */
	GenBuffers();

	/* Create underlying texture. */
	texture = new Texture(width, height, device->GetWindow(), 0, "RenderTarget");
	TextureCreationOptions opt;
	opt.MagFilter = ZoomFilter::Nearest;
	opt.MinFilter = ZoomFilter::Nearest;

	/* Set texture data for first time. */
	byte *data = malloca_s(byte, Width * Height);
	texture->SetData(data, &opt);

	/* Configure the frame buffer. */
	Configure();

	/* Reset state. */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	freea_s(data);
}

Plutonium::RenderTarget::~RenderTarget(void)
{
	delete_s(texture);
	glDeleteFramebuffers(1, &ptrFb);
	glDeleteRenderbuffers(1, &ptrDb);
}

void Plutonium::RenderTarget::GenBuffers(void)
{
	/* Generate frame buffer. */
	glGenFramebuffers(1, &ptrFb);
	glBindFramebuffer(GL_FRAMEBUFFER, ptrFb);

	/* Generate depth buffer. */
	glGenRenderbuffers(1, &ptrDb);
	glBindRenderbuffer(GL_RENDERBUFFER, ptrDb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(Width), static_cast<GLsizei>(Height));
}

void Plutonium::RenderTarget::Configure(void)
{
	/* Bind color and depth buffers. */
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ptrDb);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->ptr, 0);

	/* Set list of draw buffers. */
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);
}