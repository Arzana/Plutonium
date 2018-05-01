#include "Graphics\Diagnostics\WireframeRenderer.h"

Plutonium::WireframeRenderer::WireframeRenderer(const char * vrtxShdr)
	: beginCalled(false)
{
	/* Load shader and fields from file. */
	shdr = Shader::FromFile(vrtxShdr);

	/* Get uniforms. */
	matMdl = shdr->GetUniform("u_model");
	matView = shdr->GetUniform("u_view");
	matProj = shdr->GetUniform("u_projection");

	/* Get attribute. */
	pos = shdr->GetAttribute("a_position");
}

Plutonium::WireframeRenderer::~WireframeRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::WireframeRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	/* make sure we don't call begin twice. */
	if (!beginCalled)
	{
		/* Begin shader. */
		beginCalled = true;
		shdr->Begin();

		/* Set constant uniforms. */
		matView->Set(view);
		matProj->Set(proj);

		/* Set the polygon mode to draw wireframes. */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else LOG_WAR("Attempting to call Begin before calling End!");
}

void Plutonium::WireframeRenderer::Render(const StaticObject *model)
{
	/* Make sure begin is called and set the model matrix. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");
	matMdl->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->GetModel()->shapes.size(); i++)
	{
		/* Set mesh buffer attribute. */
		Buffer *buffer = model->GetModel()->shapes.at(i)->Mesh->GetVertexBuffer();
		buffer->Bind();
		pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::WireframeRenderer::Render(const DynamicObject * model)
{
	/* Make sure begin is called. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");

	/* Set uniforms. */
	matMdl->Set(model->GetWorld());

	/* Set first mesh buffer attributes. */
	const Mesh *curFrame = model->GetCurrentFrame();
	curFrame->GetVertexBuffer()->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render current shape. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curFrame->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::WireframeRenderer::End(void)
{
	/* Make sure end isn't called twice. */
	if (beginCalled)
	{
		/* End shader. */
		beginCalled = false;
		shdr->End();

		/* Reset polygon mode to fill. */
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else LOG_WAR("Attempting to call End before calling Begin!");
}