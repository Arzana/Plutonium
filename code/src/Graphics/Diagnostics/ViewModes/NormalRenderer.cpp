#include "Graphics\Diagnostics\ViewModes\NormalRenderer.h"

constexpr const char *VRTX_SHDR_SRC =
"#version 430 core																\n"

"uniform mat4 model;															\n"
"uniform mat4 view;																\n"
"uniform mat4 projection;														\n"

"in vec3 position;																\n"
"in vec3 normal;																\n"
"in vec3 tangent;																\n"
"in vec2 uv;																	\n"

"out vec2 textureUv;																\n"
"out mat3 tbn;																	\n"

"void main()																	\n"
"{																				\n"
"	vec3 t = normalize((model * vec4(tangent, 0.0f)).xyz);						\n"
"	vec3 n = normalize((model * vec4(normal, 0.0f)).xyz);						\n"
"	vec3 b = cross(n, t);														\n"

"	textureUv = uv;																\n"
"	tbn = mat3(t, b, n);														\n"

"	gl_Position = projection * view * model * vec4(position, 1.0f);				\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core																\n"

"uniform sampler2D bump;														\n"

"in vec2 textureUv;																\n"
"in mat3 tbn;																	\n"

"out vec4 fragColor;															\n"

"void main()																	\n"
"{																				\n"
"	vec3 rgb_normal = texture(bump, textureUv).rgb;								\n"
"	vec3 world_normal = normalize(rgb_normal * 2.0f - 1.0f);					\n"
"	world_normal = normalize(tbn * world_normal);								\n"
"	rgb_normal = normalize(world_normal * 0.5f + 0.5f);							\n"
"	fragColor = vec4(rgb_normal, 1.0f);											\n"
"}																				\n";

Plutonium::NormalRenderer::NormalRenderer(void)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC))
{
	const Shader *shdr = GetShader();

	/* Get required uniforms. */
	matMdl = shdr->GetUniform("model");
	matView = shdr->GetUniform("view");
	matProj = shdr->GetUniform("projection");
	mapBmp = shdr->GetUniform("bump");

	/* Get required attributes. */
	pos = shdr->GetAttribute("position");
	norm = shdr->GetAttribute("normal");
	tan = shdr->GetAttribute("tangent");
	uv = shdr->GetAttribute("uv");
}

void Plutonium::NormalRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	Renderer::Begin();

	matView->Set(view);
	matProj->Set(proj);
}

void Plutonium::NormalRenderer::Render(const Matrix & world, const Mesh * mesh, const Texture * bumpMap)
{
	/* Set uniforms. */
	matMdl->Set(world);
	mapBmp->Set(bumpMap);

	/* Set attributes. */
	Buffer *buffer = mesh->GetVertexBuffer();
	buffer->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));
	norm->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Normal));
	tan->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Tangent));
	uv->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Texture));

	/* Render shape. */
	DrawTris(buffer);
}