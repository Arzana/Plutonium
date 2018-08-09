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
	glBlendFuncSeparate(_CrtEnum2Int(cst), _CrtEnum2Int(cdt), _CrtEnum2Int(ast = blend), _CrtEnum2Int(adt));
}

void Plutonium::GraphicsAdapter::SetAlphaDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst), _CrtEnum2Int(cdt), _CrtEnum2Int(ast), _CrtEnum2Int(adt = blend));
}

void Plutonium::GraphicsAdapter::SetColorSourceBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst = blend), _CrtEnum2Int(cdt), _CrtEnum2Int(ast), _CrtEnum2Int(adt));
}

void Plutonium::GraphicsAdapter::SetColorDestinationBlend(BlendType blend)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst), _CrtEnum2Int(cdt = blend), _CrtEnum2Int(ast), _CrtEnum2Int(adt));
}

void Plutonium::GraphicsAdapter::SetSourceBlend(BlendType color, BlendType alpha)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst = color), _CrtEnum2Int(cdt), _CrtEnum2Int(ast = alpha), _CrtEnum2Int(adt));
}

void Plutonium::GraphicsAdapter::SetDestinationBlend(BlendType color, BlendType alpha)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst), _CrtEnum2Int(cdt = color), _CrtEnum2Int(ast), _CrtEnum2Int(adt = alpha));
}

void Plutonium::GraphicsAdapter::SetBlend(BlendType source, BlendType destination)
{
	glEnable(GL_BLEND);
	glBlendFuncSeparate(_CrtEnum2Int(cst = source), _CrtEnum2Int(cdt = destination), _CrtEnum2Int(ast = source), _CrtEnum2Int(adt = destination));
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

void Plutonium::GraphicsAdapter::SetPolygonMode(PolygonModes mode, FaceCullState face)
{
	glPolygonMode(_CrtEnum2Int(face), _CrtEnum2Int(mode));
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
		ASSERT_IF(!target->finalized, "Cannot set render target to none finalized render target!");
		glBindFramebuffer(GL_FRAMEBUFFER, target->ptrFbo);
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

Plutonium::Vector2 Plutonium::GraphicsAdapter::ToNDC(Vector2 screenCoord)
{
	Vector2 cs = window->GetClientBounds().Size;
	float x = (2.0f * screenCoord.X) / cs.X - 1.0f;
	float y = 1.0f - (2.0f * screenCoord.Y) / cs.Y;
	return Vector2(x, y);
}

Plutonium::GraphicsAdapter::GraphicsAdapter(Window * window)
	: window(window)
{
	device = _CrtGetDeviceInfo();

	/* Set defaults. */
	SetDefaultBlendEq();
	SetDefaultBlendType();
	SetDefaultStencilOp();
	UpdateBlendEq(BlendState::None);
	SetDepthTest(DepthState::LessOrEqual);

	/* Make sure pixels are tightly packed to avoid buffer overflows. */
	GLint alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	if (alignment != 1)
	{
		LOG("Unpacked pixels alignment is set to %d, resetting to 1.", alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}

	glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	if (alignment != 1)
	{
		LOG("Packed pixels alignment is set to %d, resetting to 1.", alignment);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
	}
}

void Plutonium::GraphicsAdapter::SetDefaultBlendEq(void)
{
	/* OpenGL result values. */
	GLint clrBlend, alphaBlend;

	/* Get current values from OpenGL. */
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &clrBlend);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &alphaBlend);

	/* Set them in the adapter. */
	cbf = _CrtInt2Enum<BlendState>(clrBlend);
	abf = _CrtInt2Enum<BlendState>(alphaBlend);
}

void Plutonium::GraphicsAdapter::SetDefaultBlendType(void)
{
	/* OpenGL result values. */
	GLint alphaSrc, alphaDest, clrSrc, clrDest;

	/* Get current values from OpenGL. */
	glGetIntegerv(GL_BLEND_SRC_ALPHA, &alphaSrc);
	glGetIntegerv(GL_BLEND_DST_ALPHA, &alphaDest);
	glGetIntegerv(GL_BLEND_SRC_RGB, &clrSrc);
	glGetIntegerv(GL_BLEND_DST_RGB, &clrDest);

	/* Set them in the adapter. */
	ast = _CrtInt2Enum<BlendType>(alphaSrc);
	adt = _CrtInt2Enum<BlendType>(alphaDest);
	cst = _CrtInt2Enum<BlendType>(clrSrc);
	cdt = _CrtInt2Enum<BlendType>(clrDest);
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