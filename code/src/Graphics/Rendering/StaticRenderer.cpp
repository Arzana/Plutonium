#include "Graphics\Rendering\StaticRenderer.h"

Plutonium::StaticRenderer::StaticRenderer(const char * vrtxShdr, const char * fragShdr)
	: beginCalled(false)
{
	/* Load shader and fields from files. */
	shdr = Shader::FromFile(vrtxShdr, fragShdr);

	/* Get uniforms. */
	matMdl = shdr->GetUniform("u_model");
	matView = shdr->GetUniform("u_view");
	matProj = shdr->GetUniform("u_projection");
	texture = shdr->GetUniform("u_texture");
	lightDir = shdr->GetUniform("u_light_direction");

	/* Get attributes. */
	pos = shdr->GetAttribute("a_position");
	norm = shdr->GetAttribute("a_normal");
	uv = shdr->GetAttribute("a_texture");
}

Plutonium::StaticRenderer::~StaticRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::StaticRenderer::Begin(const Matrix & view, const Matrix & proj, Vector3 lightDir)
{
	/* Make sure we don't call begin twice. */
	if (!beginCalled)
	{
		beginCalled = true;
		shdr->Begin();
		matView->Set(view);
		matProj->Set(proj);
		this->lightDir->Set(lightDir);
	}
	else LOG_WAR("Attempting to call Begin before calling End!");
}

void Plutonium::StaticRenderer::Render(const Model * model)
{
	/* Make sure begin is called and set the model matrix. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");
	matMdl->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->shapes.size(); i++)
	{
		/* Set current texture sampler. */
		Shape *cur = model->shapes.at(i);
		texture->Set(cur->Material);

		/* Set mesh buffer attributes. */
		cur->Mesh->GetVertexBuffer()->Bind();
		pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
		uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur->Mesh->GetVertexCount()));
	}
}

void Plutonium::StaticRenderer::End(void)
{
	/* Make sure end isn't called twice. */
	if (beginCalled)
	{
		beginCalled = false;
		shdr->End();
	}
	else LOG_WAR("Attempting to call End before calling Begin!");
}