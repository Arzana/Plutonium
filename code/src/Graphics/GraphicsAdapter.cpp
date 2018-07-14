#include "Graphics\GraphicsAdapter.h"

Plutonium::GraphicsAdapter::~GraphicsAdapter(void) noexcept
{
	delete_s(window);
	delete_s(device);
}

void Plutonium::GraphicsAdapter::SetAlphaBlendFunction(BlendState func)
{
	if (func != BlendState::None) abf = func;
	UpdateBlendEq(func);
}

void Plutonium::GraphicsAdapter::SetColorBlendFunction(BlendState func)
{
	if (func != BlendState::None) cbf = func;
	UpdateBlendEq(func);
}

void Plutonium::GraphicsAdapter::SetAlphaSourceBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, _CrtEnum2Int(blend));
}

void Plutonium::GraphicsAdapter::SetAlphaDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_ALPHA, _CrtEnum2Int(blend));
}

void Plutonium::GraphicsAdapter::SetColorSourceBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, _CrtEnum2Int(blend));
}

void Plutonium::GraphicsAdapter::SetColorDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, _CrtEnum2Int(blend));
}

void Plutonium::GraphicsAdapter::SetFaceCull(FaceCullState cull)
{
	if (cull == FaceCullState::None) glDisable(GL_CULL_FACE);
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(_CrtEnum2Int(cull));
	}
}

void Plutonium::GraphicsAdapter::SetFrontFace(FaceCullType func)
{
	glFrontFace(_CrtEnum2Int(func));
}

void Plutonium::GraphicsAdapter::SetStencilFailOperation(StencilOperation operation)
{
	sf = operation;
	UpdateStencilOp();
}

void Plutonium::GraphicsAdapter::SetStencilPassDepthFailOperation(StencilOperation operation)
{
	df = operation;
	UpdateStencilOp();
}

void Plutonium::GraphicsAdapter::SetStencilPassDepthPassOperation(StencilOperation operation)
{
	dp = operation;
	UpdateStencilOp();
}

void Plutonium::GraphicsAdapter::SetDepthTest(DepthState func)
{
	if (func == DepthState::None) glDisable(GL_DEPTH_TEST);
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(_CrtEnum2Int(func));
	}
}

void Plutonium::GraphicsAdapter::SetStencilTest(DepthState func, int32 value, uint32 mask)
{
	if (func == DepthState::None) glDisable(GL_STENCIL_TEST);
	else
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(_CrtEnum2Int(func), value, mask);
	}
}

void Plutonium::GraphicsAdapter::SetColorOutput(bool red, bool green, bool blue, bool alpha)
{
	glColorMask(static_cast<GLboolean>(red), static_cast<GLboolean>(green), static_cast<GLboolean>(blue), static_cast<GLboolean>(alpha));
}

void Plutonium::GraphicsAdapter::SetDepthOuput(bool mask)
{
	glDepthMask(static_cast<GLboolean>(mask));
}

void Plutonium::GraphicsAdapter::SetStencilOuput(uint32 mask)
{
	glStencilMask(mask);
}

void Plutonium::GraphicsAdapter::Clear(ClearTarget target)
{
	glClear(_CrtEnum2Int(target));
}

void Plutonium::GraphicsAdapter::SetRenderTarget(const RenderTarget * target)
{
	if (target)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, target->ptr);
		glViewport(0, 0, static_cast<GLsizei>(target->width), static_cast<GLsizei>(target->height));
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		const Rectangle vp = window->GetClientBounds();
		glViewport(static_cast<GLsizei>(vp.Position.X), static_cast<GLsizei>(vp.Position.Y), static_cast<GLsizei>(vp.GetWidth()), static_cast<GLsizei>(vp.GetHeight()));
	}
}

Plutonium::Vector2 Plutonium::GraphicsAdapter::ToOpenGL(Vector2 screenCoord)
{
	return Vector2(screenCoord.X, window->GetClientBounds().Size.Y - screenCoord.Y);
}

Plutonium::GraphicsAdapter::GraphicsAdapter(Window * window)
	: window(window)
{
	device = _CrtGetDeviceInfo();

	/* Set defaults. */
	SetDefaultBlendEq();
	SetDefaultStencilOp();
	UpdateBlendEq(BlendState::None);
	SetDepthTest(DepthState::LessOrEqual);
}

void Plutonium::GraphicsAdapter::SetDefaultBlendEq(void)
{
	/* OpenGL result values. */
	GLint clrBlend, alphaBlend;

	/* Get current values from OpenGL. */
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &clrBlend);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &alphaBlend);

	/* Set them in the adapter. */
	cbf = static_cast<BlendState>(clrBlend);
	abf = static_cast<BlendState>(alphaBlend);
}

void Plutonium::GraphicsAdapter::SetDefaultStencilOp(void)
{
	/* OpenGL result values. */
	GLint sfail, dfail, dpass;

	/* Get current values from OpenGL. */
	glGetIntegerv(GL_STENCIL_FAIL, &sfail);
	glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &dfail);
	glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &dpass);

	/* Set them in the adapter. */
	sf = static_cast<StencilOperation>(sfail);
	df = static_cast<StencilOperation>(dfail);
	dp = static_cast<StencilOperation>(dpass);
}

void Plutonium::GraphicsAdapter::UpdateBlendEq(BlendState func)
{
	/* Disable blending if requested. */
	if (func == BlendState::None) glDisable(GL_BLEND);
	else
	{
		/* Enable blending and update equation. */
		glEnable(GL_BLEND);
		glBlendEquationSeparate(_CrtEnum2Int(cbf), _CrtEnum2Int(abf));
	}
}

void Plutonium::GraphicsAdapter::UpdateStencilOp(void)
{
	/* Enable stencil and update operation. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(_CrtEnum2Int(sf), _CrtEnum2Int(df), _CrtEnum2Int(dp));
}