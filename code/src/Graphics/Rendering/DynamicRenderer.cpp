#include "Graphics\Rendering\DynamicRenderer.h"

Plutonium::DynamicRenderer::DynamicRenderer(const char * vrtxShdr, const char * fragShdr)
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
	ambient = shdr->GetUniform("u_ambient");
	time = shdr->GetUniform("u_time");

	/* Get attributes. */
	pos1 = shdr->GetAttribute("a_position_1");
	pos2 = shdr->GetAttribute("a_position_2");
	norm1 = shdr->GetAttribute("a_normal_1");
	norm2 = shdr->GetAttribute("a_normal_2");
	uv = shdr->GetAttribute("a_uv");
}

Plutonium::DynamicRenderer::~DynamicRenderer(void)
{
	delete_s(shdr);
}

void Plutonium::DynamicRenderer::Begin(const Matrix & view, const Matrix & proj, Vector3 lightDir)
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
		ambient->Set(0.5f);
		this->lightDir->Set(lightDir);
	}
	else LOG_WAR("Attempting to call Begin before calling End!");
}

void Plutonium::DynamicRenderer::Render(const DynamicObject * model)
{
	/* Make sure begin is called. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");

	/* Set uniforms. */
	matMdl->Set(model->GetWorld());
	texture->Set(model->GetModel()->skin);
	time->Set(model->mixAmnt);

	/* Set first mesh buffer attributes. */
	const Mesh *curFrame = model->GetCurrentFrame();
	curFrame->GetVertexBuffer()->Bind();
	pos1->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	norm1->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
	uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Set second mesh buffer attributes. */
	model->GetNextFrame()->GetVertexBuffer()->Bind();
	pos2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	norm2->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));

	/* Render current shape. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curFrame->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::DynamicRenderer::End(void)
{
	/* Make sure end isn't called twice. */
	if (beginCalled)
	{
		beginCalled = false;
		shdr->End();
	}
	else LOG_WAR("Attempting to call End before calling Begin!");
}