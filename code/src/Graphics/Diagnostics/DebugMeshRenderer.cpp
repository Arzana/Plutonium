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

constexpr const char *BUMP_VRTX_SHDR =
"#version 430 core																\n"

"uniform mat4 u_model;															\n"
"uniform mat4 u_view;															\n"
"uniform mat4 u_projection;														\n"

"in vec3 a_position;															\n"
"in vec3 a_normal;																\n"
"in vec3 a_tangent;																\n"
"in vec3 a_bitangent;															\n"
"in vec2 a_uv;																	\n"

"out vec2 a_texture;															\n"
"out mat3 a_tbn;																\n"

"void main()																	\n"
"{																				\n"
"	vec3 t = normalize((u_model * vec4(a_tangent, 0.0f)).xyz);					\n"
"	vec3 b = normalize((u_model * vec4(a_bitangent, 0.0f)).xyz);				\n"
"	vec3 n = normalize((u_model * vec4(a_normal, 0.0f)).xyz);					\n"

"	a_texture = a_uv;															\n"
"	a_tbn = mat3(t, b, n);														\n"

"	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);		\n"
"}";

constexpr const char *BUMP_FRAG_SHDR =
"#version 430 core																\n"

"uniform sampler2D u_bump;														\n"

"in vec2 a_texture;																\n"
"in mat3 a_tbn;																	\n"

"out vec4 fragColor;															\n"

"void main()																	\n"
"{																				\n"
"	vec3 bumpNormal = texture(u_bump, a_texture).rgb * 2.0f - 1.0f;				\n"
"	if (length(bumpNormal) > 1.0f)												\n"
"	{																			\n"
"		fragColor = vec4(a_tbn[2][0],a_tbn[2][1],a_tbn[2][2], 1.0f);			\n"
"	}																			\n"
"	else fragColor = vec4(a_tbn * normalize(bumpNormal), 1.0f);					\n"
"}																				\n";

Plutonium::DebugMeshRenderer::DebugMeshRenderer(DebuggableValues mode)
	: beginCalled(false), mode(mode)
{
	/* Load shader and fields from file. */
	shdrWf = new Shader(WIREFRAME_VRTX_SHDR);
	shdrBmp = new Shader(BUMP_VRTX_SHDR, BUMP_FRAG_SHDR);

	/* Get uniforms. */
	matMdlWf = shdrWf->GetUniform("u_model");
	matViewWf = shdrWf->GetUniform("u_view");
	matProjWf = shdrWf->GetUniform("u_projection");
	clrWf = shdrWf->GetUniform("u_color");

	matMdlBmp = shdrBmp->GetUniform("u_model");
	matViewBmp = shdrBmp->GetUniform("u_view");
	matProjBmp = shdrBmp->GetUniform("u_projection");
	mapBmp = shdrBmp->GetUniform("u_bump");

	/* Get attribute. */
	posWf = shdrWf->GetAttribute("a_position");

	posBmp = shdrBmp->GetAttribute("a_position");
	normBmp = shdrBmp->GetAttribute("a_normal");
	tanBmp = shdrBmp->GetAttribute("a_tangent");
	bitanBmp = shdrBmp->GetAttribute("a_bitangent");
	texBmp = shdrBmp->GetAttribute("a_uv");
}

Plutonium::DebugMeshRenderer::~DebugMeshRenderer(void)
{
	delete_s(shdrWf);
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
		RenderBump(model);
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
		normBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
		tanBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
		bitanBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, BiTangent));
		texBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

		/* Render current shape. */
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer->GetElementCount()));
	}
}

void Plutonium::DebugMeshRenderer::RenderBump(const DynamicObject * model)
{
	/* Set uniforms. */
	matMdlBmp->Set(model->GetWorld());

	/* Set first mesh buffer attributes. */
	const Mesh *curFrame = model->GetCurrentFrame();
	curFrame->GetVertexBuffer()->Bind();
	posBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	normBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
	tanBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
	bitanBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, BiTangent));
	texBmp->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Render current shape. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(curFrame->GetVertexBuffer()->GetElementCount()));
}

void Plutonium::DebugMeshRenderer::EndBump(void)
{
	/* End shader. */
	beginCalled = false;
	shdrBmp->End();
}