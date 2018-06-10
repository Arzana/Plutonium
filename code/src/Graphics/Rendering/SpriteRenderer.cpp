#include "Graphics\Rendering\SpriteRenderer.h"
#include "Graphics\Native\Buffer.h"
#include "Core\SafeMemory.h"

constexpr const char *VRTX_SHDR_SRC = 
"#version 430 core																			\n"

"uniform mat4 vp;																			\n"
"uniform mat4 model;																		\n"

"in vec4 posUv;																				\n"

"out vec2 uv;																				\n"

"void main()																				\n"
"{																							\n"
"	gl_Position = vp * model * vec4(posUv.xy, 0.0f, 1.0f);									\n"
"	uv = posUv.zw;																			\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core																			\n"

"uniform sampler2D sprite;																	\n"
"uniform vec4 color;																		\n"
"uniform vec4 bounds;																		\n"

"in vec2 uv;																				\n"

"out vec4 fragColor;																		\n"

"void main()																				\n"
"{																							\n"
"	if (uv.x < bounds.x || uv.x > bounds.z || uv.y < bounds.y || uv.y > bounds.w) discard;	\n"
"	fragColor = texture(sprite, uv) * color;												\n"
"}";

Plutonium::SpriteRenderer::SpriteRenderer(GraphicsAdapter * device)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC)), device(device)
{
	const Shader *shdr = GetShader();

	/* Get uniforms. */
	matVp = shdr->GetUniform("vp");
	matMdl = shdr->GetUniform("model");
	texture = shdr->GetUniform("sprite");
	color = shdr->GetUniform("color");
	bounds = shdr->GetUniform("bounds");

	/* Get attributes. */
	posUv = shdr->GetAttribute("posUv");

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

void Plutonium::SpriteRenderer::Render(const Texture * sprite, Rectangle bounds, Color color, Vector2 scale, float rotation)
{
	/* Construct model matrix. */
	const Vector2 position = device->ToOpenGL(bounds.Position);
	const Vector2 size = bounds.Size / sprite->GetSize();
	const Matrix model = Matrix::CreateWorld(position, rotation, scale);

	/* Set uniforms. */
	matMdl->Set(model);
	texture->Set(sprite);
	this->color->Set(color);
	this->bounds->Set(Vector4(0.0f, 0.0f, size.X, size.Y));

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

	mesh->SetData(vertices, 6);
}