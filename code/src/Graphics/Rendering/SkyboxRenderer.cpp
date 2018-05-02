#include "Graphics\Rendering\SkyboxRenderer.h"

using namespace Plutonium;

/* Defines the vertices that are used as a mesh by the skyboxes. */
Vector3 skyboxVertices[36] =
{
	Vector3(-1.0f,  1.0f, -1.0f),
	Vector3(-1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f,  1.0f, -1.0f),
	Vector3(-1.0f,  1.0f, -1.0f),

	Vector3(-1.0f, -1.0f,  1.0f),
	Vector3(-1.0f, -1.0f, -1.0f),
	Vector3(-1.0f,  1.0f, -1.0f),
	Vector3(-1.0f,  1.0f, -1.0f),
	Vector3(-1.0f,  1.0f,  1.0f),
	Vector3(-1.0f, -1.0f,  1.0f),

	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f,  1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(1.0f,  1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),

	Vector3(-1.0f, -1.0f,  1.0f),
	Vector3(-1.0f,  1.0f,  1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(1.0f, -1.0f,  1.0f),
	Vector3(-1.0f, -1.0f,  1.0f),

	Vector3(-1.0f,  1.0f, -1.0f),
	Vector3(1.0f,  1.0f, -1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(1.0f,  1.0f,  1.0f),
	Vector3(-1.0f,  1.0f,  1.0f),
	Vector3(-1.0f,  1.0f, -1.0f),

	Vector3(-1.0f, -1.0f, -1.0f),
	Vector3(-1.0f, -1.0f,  1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(-1.0f, -1.0f,  1.0f),
	Vector3(1.0f, -1.0f,  1.0f)
};

Plutonium::SkyboxRenderer::SkyboxRenderer(_In_ GraphicsAdapter *device, const char * vrtxShdr, const char * fragShdr)
	: device(device)
{
	/* Load shader and fields from files. */
	shdr = Shader::FromFile(vrtxShdr, fragShdr);

	/* Get uniforms. */
	matView = shdr->GetUniform("u_view");
	matProj = shdr->GetUniform("u_projection");
	texture = shdr->GetUniform("u_skybox");

	/* Get attributes. */
	pos = shdr->GetAttribute("a_position");

	/* Create single skybox VBO. */
	vbo = new Buffer(device->GetWindow());
	vbo->Bind(BindTarget::Array);
	vbo->SetData(BufferUsage::StaticDraw, skyboxVertices, sizeof(skyboxVertices) / sizeof(Vector3));
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
	pos->Initialize(false, sizeof(Vector3), offset_ptr(Vector3, X));

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