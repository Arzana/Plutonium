#include "Graphics\Rendering\SkyboxRenderer.h"
#include "Graphics\Models\Shapes.h"

using namespace Plutonium;

constexpr const char *VRTX_SHDR_SRC =
"#version 430 core													\n"

"uniform mat4 view;													\n"
"uniform mat4 projection;											\n"

"in vec3 position;													\n"

"out vec3 angle;													\n"

"void main()														\n"
"{																	\n"
"	angle = normalize(position);									\n"
"	gl_Position = (projection * view * vec4(position, 1.0f)).xyww;	\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core													\n"

"uniform samplerCube skybox;										\n"

"in vec3 angle;														\n"

"out vec4 fragColor;												\n"

"void main()														\n"
"{																	\n"
"	fragColor = texture(skybox, angle);								\n"
"}";

Plutonium::SkyboxRenderer::SkyboxRenderer(_In_ GraphicsAdapter *device)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC)), device(device)
{
	InitShader();

	/* Create single skybox VBO. */
	vbo = new Buffer(device->GetWindow(), BindTarget::Array);
	Mesh mesh("TempMesh");
	ShapeCreator::MakeBox(&mesh, Vector3::One());
	vbo->SetData(BufferUsage::StaticDraw, mesh.GetVertexAt(0), mesh.GetVertexCount());
}

Plutonium::SkyboxRenderer::~SkyboxRenderer(void)
{
	delete_s(vbo);
}

void Plutonium::SkyboxRenderer::Render(const Matrix & view, const Matrix & proj, const Texture * skybox)
{
	/* Initialize draw. */
	Begin(view, proj);
	texture->Set(skybox);
	vbo->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Position));

	/* Render skybox. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vbo->GetElementCount()));

	/* Finalize draw. */
	End();
}

void Plutonium::SkyboxRenderer::Begin(const Matrix & view, const Matrix & proj)
{
	/* Start shader and set transformations. */
	Renderer::Begin();
	matView->Set(view.GetStatic());
	matProj->Set(proj);

	/* Disable output to the depth buffer for the skybox since it will always be in the background. */
	device->SetDepthOuput(false);
}

void Plutonium::SkyboxRenderer::End(void)
{
	/* End shader and enable depth output. */
	Renderer::End();
	device->SetDepthOuput(true);
}

/* Warning cause is checked and code is working as intended. */
#pragma warning (push)
#pragma warning (disable:4458)
void Plutonium::SkyboxRenderer::InitShader(void)
{
	const Shader *shdr = GetShader();

	matView = shdr->GetUniform("view");
	matProj = shdr->GetUniform("projection");
	texture = shdr->GetUniform("skybox");
	pos = shdr->GetAttribute("position");
}
#pragma warning(pop)