#include "Graphics\Diagnostics\DebugMeshRenderer.h"

constexpr const char *WIREFRAME_VRTX_SHDR =
"#version 430 core																\n"

"uniform mat4 u_model;															\n"
"uniform mat4 u_view;															\n"
"uniform mat4 u_projection;														\n"
"uniform vec4 u_color;															\n"

"in vec3 a_position;															\n"

"void main()																	\n"
"{																				\n"
"	gl_FrontColor = u_color;													\n"
"	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);		\n"
"}";

constexpr const char *NORMALS_VRTX_SHDR =
"#version 430 core																\n"

"uniform mat4 u_model;															\n"
"uniform mat4 u_view;															\n"
"uniform mat4 u_projection;														\n"

"in vec3 a_position;															\n"
"in vec3 a_normal;																\n"

"void main()																	\n"
"{																				\n"
"	gl_FrontColor = vec4(a_normal, 1.0f);										\n"
"	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);		\n"
"}";

constexpr const char *BUMP_VRTX_SHDR =
"#version 430 core																\n"

"uniform mat4 u_model;															\n"
"uniform mat4 u_view;															\n"
"uniform mat4 u_projection;														\n"
"uniform sampler2D u_map;														\n"

"in vec3 a_position;															\n"
"in vec2 a_texture;																\n"

"void main()																	\n"
"{																				\n"
"	gl_FrontColor = texture(u_map, a_texture);									\n"
"	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);		\n"
"}";

Plutonium::DebugMeshRenderer::DebugMeshRenderer(DebuggableValues mode)
	: beginCalled(false), mode(mode)
{
	/* Load shader and fields from file. */
	shdrWf = new Shader(WIREFRAME_VRTX_SHDR);
	shdrN = new Shader(NORMALS_VRTX_SHDR);
	shdrBmp = new Shader(BUMP_VRTX_SHDR);

	/* Get uniforms. */
	matMdlWf = shdrWf->GetUniform("u_model");
	matViewWf = shdrWf->GetUniform("u_view");
	matProjWf = shdrWf->GetUniform("u_projection");
	clrWf = shdrWf->GetUniform("u_color");

	matMdlN = shdrN->GetUniform("u_model");
	matViewN = shdrN->GetUniform("u_view");
	matProjN = shdrN->GetUniform("u_projection");

	matMdlBmp = shdrBmp->GetUniform("u_model");
	matViewBmp = shdrBmp->GetUniform("u_view");
	matProjBmp = shdrBmp->GetUniform("u_projection");
	mapBmp = shdrBmp->GetUniform("u_map");

	/* Get attribute. */
	posWf = shdrWf->GetAttribute("a_position");

	posN = shdrN->GetAttribute("a_position");
	normN = shdrN->GetAttribute("a_normal");

	posBmp = shdrBmp->GetAttribute("a_position");
	texBmp = shdrBmp->GetAttribute("a_texture");
}

Plutonium::DebugMeshRenderer::~DebugMeshRenderer(void)
{
	delete_s(shdrWf);
	delete_s(shdrN);
	delete_s(shdrBmp);
}

void Plutonium::DebugMeshRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	/* make sure we don't call begin twice. */
	if (!beginCalled)
	{
		switch (mode)
		{
		case DebuggableValues::Wireframe:
			BeginWireframe(view, proj);
			break;
		case DebuggableValues::Normals:
			BeginNormals(view, proj);
			break;
		case DebuggableValues::Bump:
			BeginBump(view, proj);
			break;
		default:
			LOG_THROW("Begin not defined for value!");
			break;
		}
	}
	else LOG_WAR("Attempting to call Begin before calling End!");
}

void Plutonium::DebugMeshRenderer::Render(const StaticObject *model, Color color)
{
	/* Make sure begin is called and set the model matrix. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");

	switch (mode)
	{
	case DebuggableValues::Wireframe:
		RenderWireframe(model, color);
		break;
	case DebuggableValues::Normals:
		RenderNormals(model);
		break;
	case DebuggableValues::Bump:
		RenderBump(model);
		break;
	default:
		LOG_THROW("Render not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::Render(const DynamicObject * model, Color color)
{
	/* Make sure begin is called. */
	ASSERT_IF(!beginCalled, "Cannot call Render before calling Begin!");

	switch (mode)
	{
	case DebuggableValues::Wireframe:
		RenderWireframe(model, color);
		break;
	case DebuggableValues::Normals:
		RenderNormals(model);
		break;
	case DebuggableValues::Bump:
		LOG_WAR_ONCE("Cannot render dynamic model as a bump map at this point!");
		break;
	default:
		LOG_THROW("Render not defined for value!");
		break;
	}
}

void Plutonium::DebugMeshRenderer::End(void)
{
	/* Make sure end isn't called twice. */
	if (beginCalled)
	{
		switch (mode)
		{
		case DebuggableValues::Wireframe:
			EndWireframe();
			break;
		case DebuggableValues::Normals:
			EndNormals();
			break;
		case DebuggableValues::Bump:
			EndBump();
			break;
		default:
			LOG_THROW("End not defined for value!");
			break;
		}
	}
	else LOG_WAR("Attempting to call End before calling Begin!");
}

void Plutonium::DebugMeshRenderer::BeginWireframe(const Matrix & view, const Matrix & proj)
{
	/* Begin shader. */
	beginCalled = true;
	shdrWf->Begin();

	/* Set constant uniforms. */
	matViewWf->Set(view);
	matProjWf->Set(proj);

	/* Set the polygon mode to draw wireframes. */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Plutonium::DebugMeshRenderer::RenderWireframe(const StaticObject * model, Color color)
{
	matMdlWf->Set(model->GetWorld());
	clrWf->Set(color);

	/* Render each shape. */
	for (size_t i = 0; i < model->GetModel()->shapes.size(); i++)
	{
		/* Set mesh buffer attribute. */
		Buffer *buffer = model->GetModel()->shapes.at(i)->Mesh->GetVertexBuffer();
		buffer->Bind();
		posWf->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::DebugMeshRenderer::RenderWireframe(const DynamicObject * model, Color color)
{
	/* Set uniforms. */
	matMdlWf->Set(model->GetWorld());
	clrWf->Set(color);

	/* Set first mesh buffer attributes. */
	const Mesh *curFrame = model->GetCurrentFrame();
	curFrame->GetVertexBuffer()->Bind();
	posWf->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render current shape. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curFrame->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::DebugMeshRenderer::EndWireframe(void)
{
	/* End shader. */
	beginCalled = false;
	shdrWf->End();

	/* Reset polygon mode to fill. */
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Plutonium::DebugMeshRenderer::BeginNormals(const Matrix & view, const Matrix & proj)
{
	/* Begin shader. */
	beginCalled = true;
	shdrN->Begin();

	/* Set constant uniforms. */
	matViewN->Set(view);
	matProjN->Set(proj);
}

void Plutonium::DebugMeshRenderer::RenderNormals(const StaticObject * model)
{
	matMdlN->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->GetModel()->shapes.size(); i++)
	{
		/* Set mesh buffer attribute. */
		Buffer *buffer = model->GetModel()->shapes.at(i)->Mesh->GetVertexBuffer();
		buffer->Bind();
		posN->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		normN->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::DebugMeshRenderer::RenderNormals(const DynamicObject * model)
{
	/* Set uniforms. */
	matMdlN->Set(model->GetWorld());

	/* Set first mesh buffer attributes. */
	const Mesh *curFrame = model->GetCurrentFrame();
	curFrame->GetVertexBuffer()->Bind();
	posN->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	normN->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));

	/* Render current shape. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curFrame->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::DebugMeshRenderer::EndNormals(void)
{
	/* End shader. */
	beginCalled = false;
	shdrN->End();
}

void Plutonium::DebugMeshRenderer::BeginBump(const Matrix & view, const Matrix & proj)
{
	/* Begin shader. */
	beginCalled = true;
	shdrBmp->Begin();

	/* Set constant uniforms. */
	matViewBmp->Set(view);
	matProjBmp->Set(proj);
}

void Plutonium::DebugMeshRenderer::RenderBump(const StaticObject * model)
{
	matMdlBmp->Set(model->GetWorld());

	/* Render each shape. */
	for (size_t i = 0; i < model->GetModel()->shapes.size(); i++)
	{
		/* Set bump map. */
		PhongShape *cur = model->GetModel()->shapes.at(i);
		mapBmp->Set(cur->BumpMap);

		/* Set mesh buffer attribute. */
		Buffer *buffer = cur->Mesh->GetVertexBuffer();
		buffer->Bind();
		posBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
		texBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::DebugMeshRenderer::EndBump(void)
{
	/* End shader. */
	beginCalled = false;
	shdrBmp->End();
}