#include "Graphics\Portals\PortalRenderer.h"

Plutonium::PortalRenderer::PortalRenderer(GraphicsAdapter * device, const char * vrtxShdr, const char * fragShdr)
	: device(device), drawNonPortals(nullptr)
{
	/* Load shader and fields from files. */
	shdr = Shader::FromFile(vrtxShdr, fragShdr);

	/* Get uniforms. */
	matMdl = shdr->GetUniform("u_model");
	matView = shdr->GetUniform("u_view");
	matProj = shdr->GetUniform("u_projection");

	/* Get attributes. */
	pos = shdr->GetAttribute("a_position");
}

Plutonium::PortalRenderer::~PortalRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::PortalRenderer::Render(const Matrix & view, const Matrix & proj)
{
	Begin();
	RenderPortals(view, proj, 0);
	End();
}

void Plutonium::PortalRenderer::Begin(void)
{
	/* Begin shader and set light uniform. */
	shdr->Begin();

	/* Disable color and depth output. */
	device->SetDepthTest(DepthState::None);
	device->SetColorOutput(false, false, false, false);
	device->SetDepthOuput(false);

	/* Increase stencil on area of inner portal. */
	device->SetStencilOuput(0xFF);
	device->SetStencilFailOperation(StencilOperation::Increase);
}

void Plutonium::PortalRenderer::RenderPortals(const Matrix & view, const Matrix & proj, int32 curRecusrionDepth)
{
	/* Set stencil depth func. */
	device->SetStencilTest(DepthState::Differ, curRecusrionDepth);

	for (size_t i = 0; i < portals.size(); i++)
	{
		/* Render the current portal. */
		Portal *cur = portals.at(i);
		RenderSinglePortal(cur, view, proj);

		/* Get the new view and projection matrices. */
		Matrix destView = cur->GetInverseView(view);
		Matrix destProj = cur->GetClippedProjection(view, proj);

		if (curRecusrionDepth >= maxRecursionDepth)
		{
			/* Enable color and depth output. */
			device->SetDepthTest(DepthState::LessOrEqual);
			device->SetColorOutput(true, true, true, true);
			device->SetDepthOuput(true);

			/* Clear depth buffer. */
			device->Clear(ClearTarget::Depth);

			/* Disable rendering to stencil buffer but enable test. */
			device->SetStencilOuput(0x00);
			device->SetStencilTest(DepthState::LessOrEqual, curRecusrionDepth + 1);

			/* Render scene limited to the stencil buffer. */
			if (drawNonPortals) drawNonPortals(destView, destProj);
		}
		else RenderPortals(destView, destProj, curRecusrionDepth + 1);

		/* Disable color and depth output. */
		device->SetColorOutput(false, false, false, false);
		device->SetDepthOuput(false);

		/* Enable stencil test and output. */
		device->SetStencilOuput(0xFF);
		device->SetStencilTest(DepthState::Differ, curRecusrionDepth + 1);
		device->SetStencilFailOperation(StencilOperation::Decrease);

		/* Render current portal. */
		RenderSinglePortal(cur, view, proj);
	}

	/* Disable stencil output. */
	device->SetStencilTest(DepthState::None);
	device->SetStencilOuput(0x00);

	/* Disable color output. */
	device->SetColorOutput(false, false, false, false);

	/* Enable depth test. */
	device->SetDepthOuput(true);
	device->SetDepthTest(DepthState::Always);

	/* Clear buffer. */
	device->Clear(ClearTarget::Depth);

	/* Render portals to depth buffer. */
	for (size_t i = 0; i < portals.size(); i++)
	{
		RenderSinglePortal(portals.at(i), view, proj);
	}

	/* Enable stencil test. */
	device->SetStencilTest(DepthState::LessOrEqual, curRecusrionDepth);
	device->SetStencilOuput(0x00);

	/* Enable depth and color output.. */
	device->SetDepthTest(DepthState::LessOrEqual);
	device->SetColorOutput(true, true, true, true);
	device->SetDepthOuput(true);

	/* Render scene normally. */
	if (drawNonPortals) drawNonPortals(view, proj);
}

void Plutonium::PortalRenderer::RenderSinglePortal(Portal * portal, const Matrix & view, const Matrix & proj)
{
	/* Set uniforms. */
	matProj->Set(proj);
	matView->Set(view);
	matMdl->Set(portal->GetWorld());

	/* Bind buffer and set position attribute. */
	Buffer *buffer = portal->mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render portal frame to stencil buffer. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
}

void Plutonium::PortalRenderer::End(void)
{
	/* End shader. */
	shdr->End();

	/* Re-enable color and depth output. */
	device->SetColorOutput(true, true, true, true);
	device->SetDepthOuput(true);

	/* Disable stencil test. */
	device->SetStencilOuput(0x00);
	device->SetStencilTest(DepthState::None);
}