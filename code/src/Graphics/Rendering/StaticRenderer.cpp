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

	mapAmbi = shdr->GetUniform("u_texture_ambient");
	mapDiff = shdr->GetUniform("u_texture_diffuse");
	mapSpec = shdr->GetUniform("u_texture_specular");
	mapAlpha = shdr->GetUniform("u_texture_alpha");

	lightDir = shdr->GetUniform("u_light_direction");
	lightClr = shdr->GetUniform("u_light_color");
	specExp = shdr->GetUniform("u_spec_exp");
	camPos = shdr->GetUniform("u_view_pos");

	filter = shdr->GetUniform("u_frag_filter");
	ambient = shdr->GetUniform("u_refl_ambient");
	diffuse = shdr->GetUniform("u_refl_diffuse");
	specular = shdr->GetUniform("u_refl_specular");

	/* Get attributes. */
	pos = shdr->GetAttribute("a_position");
	norm = shdr->GetAttribute("a_normal");
	uv = shdr->GetAttribute("a_uv");
}

Plutonium::StaticRenderer::~StaticRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::StaticRenderer::Begin(const Matrix & view, const Matrix & proj, Vector3 camPos, Vector3 lightDir, Color lightClr)
{
	/* Make sure we don't call begin twice. */
	if (!beginCalled)
	{
		/* Begin shader. */
		beginCalled = true;
		shdr->Begin();

		/* Set constant uniforms. */
		matView->Set(view);
		matProj->Set(proj);
		this->camPos->Set(camPos);
		this->lightDir->Set(lightDir);
		this->lightClr->Set(lightClr);
	}
	else LOG_WAR("Attempting to call Begin before calling End!");
}

void Plutonium::StaticRenderer::Render(const StaticModel * model)
{
	/* Make sure begin is called and set the model matrix. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");
	matMdl->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->shapes.size(); i++)
	{
		/* Get current textured mesh. */
		PhongShape *cur = model->shapes.at(i);
		Buffer *buffer = cur->Mesh->GetVertexBuffer();

		/* Set material attributes. */
		mapAmbi->Set(cur->AmbientMap);
		mapDiff->Set(cur->DiffuseMap);
		mapSpec->Set(cur->SpecularMap);
		mapAlpha->Set(cur->AlphaMap);
		specExp->Set(cur->SpecularExp);
		filter->Set(cur->Transmittance);
		ambient->Set(cur->Ambient);
		diffuse->Set(cur->Diffuse);
		specular->Set(cur->Specular);

		/* Set mesh buffer attributes. */
		buffer->Bind();
		pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
		uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::StaticRenderer::Render(const EuclidRoom * model)
{
	/* Make sure begin is called and set the model matrix. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");
	matMdl->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->shapes.size(); i++)
	{
		/* Set current texture sampler. */
		PhongShape *cur = model->shapes.at(i);
		mapDiff->Set(cur->DiffuseMap);

		/* Set mesh buffer attributes. */
		cur->Mesh->GetVertexBuffer()->Bind();
		pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
		uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cur->Mesh->GetVertexBuffer()->GetElementCount()));
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