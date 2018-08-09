#include "Graphics\Rendering\ShapeRenderer.h"

constexpr const char *VRTX_SHDR_SRC =
"#version 430 core											\n"

"uniform mat4 View;											\n"
"uniform mat4 Projection;									\n"

"in vec3 Position;											\n"
"in vec4 Color;												\n"

"out vec4 LineColor;										\n"

"void main()												\n"
"{															\n"
"	LineColor = Color;										\n"
"	gl_Position = Projection * View * vec4(Position, 1.0f);	\n"
"}";

constexpr const char *FRAG_SHDR_SRC =
"#version 430 core											\n"

"in vec4 LineColor;											\n"

"out vec4 FragColor;										\n"

"void main()												\n"
"{															\n"
"	FragColor = LineColor;									\n"
"}";

Plutonium::ShapeRenderer::ShapeRenderer(const GraphicsAdapter * device, size_t bufferSize)
	: Renderer(new Shader(VRTX_SHDR_SRC, FRAG_SHDR_SRC)), device(device),
	lineCnt(0), maxLineCnt(bufferSize)
{
	InitShader();

	queue = malloc_s(VertexFormat, bufferSize << 1);
	AllocVbo();
}

Plutonium::ShapeRenderer::~ShapeRenderer(void)
{
	free_s(queue);
	delete_s(vbo);
}

void Plutonium::ShapeRenderer::AddRay(Vector3 start, Vector3 end, Color clr)
{
	/* Increase the GPU and CPU buffer if needed. */
	if (lineCnt >= maxLineCnt)
	{
		maxLineCnt <<= 1;
		realloc_s(VertexFormat, queue, maxLineCnt << 1);
		delete_s(vbo);
		AllocVbo();
	}

	/* Push first vertex. */
	queue[lineCnt << 1].Pos = start;
	queue[lineCnt << 1].Clr = clr.ToVector4();

	/* Push second vertex. */
	queue[(lineCnt << 1) + 1].Pos = end;
	queue[(lineCnt << 1) + 1].Clr = clr.ToVector4();

	++lineCnt;
}

void Plutonium::ShapeRenderer::AddCircle(Vector3 center, float radius, Color clr, int32 divs)
{
	float delta = PI / static_cast<float>(divs);

	Vector3 v0 = center + Vector3(radius, 0.0f, 0.0f);
	for (float theta = delta; theta < TAU + delta; theta += delta)
	{
		Vector3 v1 = center + Vector3(cosf(theta), 0.0f, sinf(theta)) * radius;
		AddRay(v0, v1, clr);
		v0 = v1;
	}
}

void Plutonium::ShapeRenderer::AddSphere(Vector3 center, float radius, Color xzClr, Color xyClr, Color yzClr, int32 divs)
{
	float delta = PI / static_cast<float>(divs);

	Vector3 v0x = center + Vector3(radius, 0.0f, 0.0f);
	Vector3 v0y = center + Vector3(radius, 0.0f, 0.0f);
	Vector3 v0z = center + Vector3(0.0f, 0.0f, radius);

	for (float theta = delta; theta < TAU + delta; theta += delta)
	{
		float c = cosf(theta);
		float s = sinf(theta);

		Vector3 v1x = center + Vector3(c, 0.0f, s) * radius;
		Vector3 v1y = center + Vector3(c, s, 0.0f) * radius;
		Vector3 v1z = center + Vector3(0.0f, s, c) * radius;

		AddRay(v0x, v1x, xzClr);
		AddRay(v0y, v1y, xyClr);
		AddRay(v0z, v1z, yzClr);

		v0x = v1x;
		v0y = v1y;
		v0z = v1z;
	}
}

void Plutonium::ShapeRenderer::AddBox(const Box & box, Color clr)
{
	Vector3 ftr = box.Position + Vector3(box.Size.X, 0.0f, 0.0f);
	Vector3 fbr = box.Position + Vector3(box.Size.X, box.Size.Y, 0.0f);
	Vector3 fbl = box.Position + Vector3(0.0f, box.Size.Y, 0.0f);
	Vector3 btl = box.Position + Vector3(0.0f, 0.0f, box.Size.Z);
	Vector3 btr = box.Position + Vector3(box.Size.X, 0.0f, box.Size.Z);
	Vector3 bbr = box.Position + box.Size;
	Vector3 bbl = box.Position + Vector3(0.0f, box.Size.Y, box.Size.Z);

	AddRay(box.Position, ftr, clr);
	AddRay(ftr, fbr, clr);
	AddRay(fbr, fbl, clr);
	AddRay(fbl, box.Position, clr);

	AddRay(btl, btr, clr);
	AddRay(btr, bbr, clr);
	AddRay(bbr, bbl, clr);
	AddRay(bbl, btl, clr);

	AddRay(box.Position, btl, clr);
	AddRay(ftr, btr, clr);
	AddRay(fbr, bbr, clr);
	AddRay(fbl, bbl, clr);
}

void Plutonium::ShapeRenderer::Render(const Matrix & view, const Matrix & projection)
{
	/* Push lines to GPU. */
	vbo->SetData(queue, lineCnt << 1);

	/* Set shader globals. */
	Begin();
	matProj->Set(projection);
	matView->Set(view);

	/* Render lines. */
	vbo->Bind();
	pos->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Pos));
	clr->Initialize(false, sizeof(VertexFormat), offset_ptr(VertexFormat, Clr));
	glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vbo->GetElementCount()));

	/* Reset the buffer. */
	End();
	lineCnt = 0;
}

void Plutonium::ShapeRenderer::AllocVbo(void)
{
	vbo = new Buffer(device->GetWindow(), BindTarget::Array);
	vbo->SetData<VertexFormat>(BufferUsage::DynamicDraw, nullptr, maxLineCnt << 1);
}

void Plutonium::ShapeRenderer::InitShader(void)
{
	const Shader *shdr = GetShader();

	matProj = shdr->GetUniform("Projection");
	matView = shdr->GetUniform("View");
	pos = shdr->GetAttribute("Position");
	clr = shdr->GetAttribute("Color");
}