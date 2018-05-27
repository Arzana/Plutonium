#include "Graphics\Diagnostics\ViewModes\UnlitRenderer.h"

constexpr const char *VRTX_SHDR_SRC =
"#version 430 core														\n"

"uniform mat4 model;													\n"
"uniform mat4 view;														\n"
"uniform mat4 projection;												\n"

"in vec3 position;														\n"
"in vec2 uv;															\n"

"out vec2 textureUv;													\n"

"void main()															\n"
"{																		\n"
"	textureUv = uv;														\n"
"	gl_Position = projection * view * model * vec4(position, 1.0f);		\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core														\n"

"uniform sampler2D base;												\n"
"uniform sampler2D alpha;												\n"

"in vec2 textureUv;														\n"

"out vec4 fragColor;													\n"

"void main()															\n"
"{																		\n"
"	fragColor = texture(base, textureUv) * texture(alpha, textureUv);	\n"
"}";

Plutonium::UnlitRenderer::UnlitRenderer(void)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC))
{
	const Shader *shdr = GetShader();

	/* Get uniforms. */
	matmdl = shdr->GetUniform("model");
	matView = shdr->GetUniform("view");
	matProj = shdr->GetUniform("projection");
	mapBase = shdr->GetUniform("base");
	mapAlpha = shdr->GetUniform("alpha");

	/* Get attributes. */
	pos = shdr->GetAttribute("position");
	uv = shdr->GetAttribute("uv");
}

void Plutonium::UnlitRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	Renderer::Begin();

	matView->Set(view);
	matProj->Set(proj);
}

void Plutonium::UnlitRenderer::Render(const Matrix & world, const Mesh * mesh, const Texture * base, const Texture * alpha)
{
	/* Set uniforms. */
	matmdl->Set(world);
	mapBase->Set(base);
	mapAlpha->Set(alpha);

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Render shape. */
	DrawTris(buffer);
}