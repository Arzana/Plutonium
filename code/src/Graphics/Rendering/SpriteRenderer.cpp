#include "Graphics\Rendering\SpriteRenderer.h"
#include "Graphics\Native\Buffer.h"
#include "Core\SafeMemory.h"

Plutonium::SpriteRenderer::SpriteRenderer(GraphicsAdapter * device, const char * vrtxShdr, const char * fragShdr)
	: Renderer(Shader::FromFile(vrtxShdr, fragShdr)), device(device)
{
	const Shader *shdr = GetShader();

	/* Get uniforms. */
	matVp = shdr->GetUniform("u_vp");
	matMdl = shdr->GetUniform("u_model");
	texture = shdr->GetUniform("u_texture");
	color = shdr->GetUniform("u_color");

	/* Get attributes. */
	posUv = shdr->GetAttribute("a_pos_uv");

	/* Make sure projection matrix is updated on window resize. */
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
	device->GetWindow()->SizeChanged.Add(this, &SpriteRenderer::WindowResizeEventHandler);

	/* Create VBO for sprite mesh. */
	mesh = new Buffer(device->GetWindow(), BindTarget::Array);
	mesh->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, 6);
}

Plutonium::SpriteRenderer::~SpriteRenderer(void)
{
	/* Remove event handler. */
	device->GetWindow()->SizeChanged.Remove(this, &SpriteRenderer::WindowResizeEventHandler);

	delete_s(mesh);
}

void Plutonium::SpriteRenderer::Begin(void)
{
	Renderer::Begin();

	/* Set constant uniforms. */
	matVp->Set(proj);
}

void Plutonium::SpriteRenderer::Render(const Texture * sprite, Vector2 position, Color color, Vector2 scale, float rotation)
{
	/* Construct model matrix. */
	position = device->ToOpenGL(position);
	Matrix model = Matrix::CreateTranslation(position.X, position.Y, 0.0f)
		* Matrix::CreateRotationZ(rotation)
		* Matrix::CreateScalar(scale.X, scale.Y, 1.0f);

	/* Set uniforms. */
	matMdl->Set(model);
	texture->Set(sprite);
	this->color->Set(color);

	/* Set mesh buffer. */
	UpdateVBO(sprite->GetSize() * scale);
	posUv->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

	/* Render sprite. */
	DrawTris(mesh->GetElementCount());
}

void Plutonium::SpriteRenderer::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	/* Update projection matrix. */
	Rectangle viewport = sender->GetClientBounds();
	proj = Matrix::CreateOrtho(viewport.Position.X, viewport.Size.X, viewport.Position.Y, viewport.Size.Y, 0.0f, 1.0f);
}

void Plutonium::SpriteRenderer::UpdateVBO(Vector2 size)
{
	// Clockwise front face.
	Vector4 vertices[6]
	{
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),		// bottom-left
		Vector4(0.0f, -size.Y, 0.0f, 0.0f),		// top-left
		Vector4(size.X, -size.Y, 1.0f, 0.0f),	// top-right
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),		// bottom-left
		Vector4(size.X, -size.Y, 1.0f, 0.0f),	// top-right
		Vector4(size.X, 0.0f, 1.0f, 1.0f)		// bottom-right
	};

	mesh->Bind();
	mesh->SetData(vertices, 6);
}