#include "Graphics\GraphicsAdapter.h"

Plutonium::GraphicsAdapter::~GraphicsAdapter(void) noexcept
{
	delete_s(window);
}

void Plutonium::GraphicsAdapter::SetAlphaBlendFunction(BlendState func)
{
	UpdateBlendEq(abf = func);
}

void Plutonium::GraphicsAdapter::SetColorBlendFunction(BlendState func)
{
	UpdateBlendEq(cbf = func);
}

void Plutonium::GraphicsAdapter::SetAlphaSourceBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, static_cast<GLenum>(blend));
}

void Plutonium::GraphicsAdapter::SetAlphaDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, static_cast<GLenum>(blend));
}

void Plutonium::GraphicsAdapter::SetColorSourceBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, static_cast<GLenum>(blend));
}

void Plutonium::GraphicsAdapter::SetColorDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, static_cast<GLenum>(blend));
}

void Plutonium::GraphicsAdapter::SetDepthTest(DepthState func)
{
	if (func == DepthState::None) glDisable(GL_BLEND);
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(static_cast<GLenum>(func));
	}
}

Plutonium::GraphicsAdapter::GraphicsAdapter(Window * window)
	: window(window),								// Set window and depth.
	cbf(BlendState::None), abf(BlendState::None)	// Set blend equations.
{
	/* Set defaults. */
	UpdateBlendEq(BlendState::None);
	SetDepthTest(DepthState::LessOrEqual);
}

void Plutonium::GraphicsAdapter::UpdateBlendEq(BlendState func)
{
	/* Disable blending if requested. */
	if (func == BlendState::None) glDisable(GL_BLEND);
	else
	{
		/* Enable blending and update equation. */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(static_cast<GLenum>(cbf), static_cast<GLenum>(abf));
	}
}