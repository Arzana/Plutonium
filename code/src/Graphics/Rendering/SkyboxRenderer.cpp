#include "Graphics\Rendering\SkyboxRenderer.h"
#include "Graphics\Models\Shapes.h"

using namespace Plutonium;

Plutonium::SkyboxRenderer::SkyboxRenderer(_In_ GraphicsAdapter *device)
	: device(device)
{
	InitShader();

	/* Create single skybox VBO. */
	vbo = new Buffer(device->GetWindow(), BindTarget::Array);
	Mesh mesh("TempMesh");
	ShapeCreator::MakeBox(&mesh);
	vbo->SetData(BufferUsage::StaticDraw, mesh.GetVertexAt(0), mesh.GetVertexCount());
}

Plutonium::SkyboxRenderer::~SkyboxRenderer(void)
{
	delete_s(shdr);
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
	shdr->Begin();
	matView->Set(view.GetStatic());
	matProj->Set(proj);

	/* Disable output to the depth buffer for the skybox since it will always be in the background. */
	device->SetDepthOuput(false);
}

void Plutonium::SkyboxRenderer::End(void)
{
	/* End shader and enable depth output. */
	shdr->End();
	device->SetDepthOuput(true);
}

void Plutonium::SkyboxRenderer::InitShader(void)
{
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

	shdr = new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC);
	matView = shdr->GetUniform("view");
	matProj = shdr->GetUniform("projection");
	texture = shdr->GetUniform("skybox");
	pos = shdr->GetAttribute("position");
}