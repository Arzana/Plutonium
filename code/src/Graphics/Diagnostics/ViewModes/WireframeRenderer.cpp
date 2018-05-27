#include "Graphics\Diagnostics\ViewModes\WireframeRenderer.h"

constexpr const char *VRTX_SHDR_SRC =
"#version 430 core														\n"

"uniform mat4 model;													\n"
"uniform mat4 view;														\n"
"uniform mat4 projection;												\n"
"uniform vec4 color;													\n"

"in vec3 position;														\n"

"out vec3 clr;															\n"

"void main()															\n"
"{																		\n"
"	clr = color.rgb;													\n"
"	gl_Position = projection * view * model * vec4(position, 1.0);		\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core														\n"

"in vec3 clr;															\n"

"out vec4 fragColor;													\n"

"void main()															\n"
"{																		\n"
"	fragColor = vec4(clr, 1.0f);										\n"
"}";


Plutonium::WireframeRenderer::WireframeRenderer(void)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC))
{
	const Shader *shdr = GetShader();

	/* Get required uniforms. */
	matMdl = shdr->GetUniform("model");
	matView = shdr->GetUniform("view");
	matProj = shdr->GetUniform("projection");
	clr = shdr->GetUniform("color");

	/* Get required attributes. */
	pos = shdr->GetAttribute("position");
}

void Plutonium::WireframeRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	Renderer::Begin();

	/* Set view parameters. */
	matView->Set(view);
	matProj->Set(proj);

	/* Set the polygon mode to draw wireframes. */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void Plutonium::WireframeRenderer::Render(const Matrix & world, const Mesh * mesh, Color color)
{
	/* Set uniforms. */
	matMdl->Set(world);
	clr->Set(color);

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render shape. */
	DrawTris(buffer);
}

void Plutonium::WireframeRenderer::End(void)
{
	Renderer::End();

	/* Reset polygon mode to fill. */
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}