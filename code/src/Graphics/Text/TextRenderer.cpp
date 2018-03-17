#include "Graphics\Text\TextRenderer.h"
#include "Core\StringFunctions.h"

constexpr Plutonium::int32 MAX_STRING_LENGTH = 64;

Plutonium::FontRenderer::FontRenderer(GraphicsAdapter *device, const char * font, const char * vrtxShdr, const char * fragShdr)
	: device(device)
{
	/* Load shader and fields from files. */
	shdr = Shader::FromFile(vrtxShdr, fragShdr);
	clr = shdr->GetUniform("u_color");
	tex = shdr->GetUniform("u_texture");
	wvp = shdr->GetUniform("u_wvp");
	pos = shdr->GetAttribute("a_position");

	/* Load font from file. */
	this->font = Font::FromFile(font, 24.0f);

	/* Make sure projection matrix is updated on window resize. */
	WindowResizeEventHandler(device->GetWindow(), EventArgs());
	device->GetWindow()->SizeChanged.Add(this, &FontRenderer::WindowResizeEventHandler);

	vbo = new Buffer();
	vbo->Bind(BindTarget::Array);
	vbo->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, MAX_STRING_LENGTH * 6);
}

Plutonium::FontRenderer::~FontRenderer(void)
{
	/* Remove event handler. */
	device->GetWindow()->SizeChanged.Remove(this, &FontRenderer::WindowResizeEventHandler);

	/* Delete shader and font. */
	delete_s(vbo);
	delete_s(shdr);
	delete_s(font);
}

void Plutonium::FontRenderer::AddString(Vector2 pos, const char * str)
{
	/* Initialize newline split buffer. */
	constexpr size_t BUFF_LEN = 16;
	char *buffer[BUFF_LEN];
	for (size_t i = 0; i < BUFF_LEN; i++) buffer[i] = malloca_s(char, FILENAME_MAX);

	/* Split string to lines and loop through them. */
	size_t len = spltstr(str, '\n', buffer, 0);
	for (size_t i = 0; i < len; i++)
	{
		AddSingleString(Vector2(pos.X, pos.Y - font->lineSpace * i), buffer[i]);
	}
}

void Plutonium::FontRenderer::Render(void)
{
	/* If there are strings render them. */
	if (strs.size() > 0)
	{
		/* Begin shader and set current projection matrix. */
		shdr->Begin();
		wvp->Set(proj);
		tex->Set(font->map);

		/* Make sure blending is enabled. */
		device->SetAlphaSourceBlend(BlendType::ISrcAlpha);

		/* Render all stored strings. */
		for (size_t i = 0; i < strs.size(); i++)
		{
			RenderString(strs.at(i), vrtxs.at(i), Color::White);
		}

		/* End shader and clear buffers. */
		shdr->End();
		ClearBuffer();
	}
}

void Plutonium::FontRenderer::RenderString(const char * string, Vector2 pos, Color clr)
{
	/* Update buffer. */
	UpdateVBO(pos, string);

	/* Set color and position parameters. */
	this->clr->Set(clr);
	this->pos->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

	/* Render line. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vbo->GetElementCount()));
}

void Plutonium::FontRenderer::UpdateVBO(Vector2 pos, const char *str)
{
	/* Create CPU side vertices buffer. (6 vertices per quad of vector4 type per glyph). */
	size_t size = strlen(str) * 6;
	Vector4 *vertices = malloc_s(Vector4, size);

	for (size_t i = 0, j = 0; i < (size / 6); i++)
	{
		/* Get current character and set texture. */
		Character *ch = font->GetCharOrDefault(str[i]);

		/* Defines components of the position vertices. */
		float x = pos.X + ch->Bearing.X;
		float y = pos.Y - (ch->Size.Y + ch->Bearing.Y);
		float w = ch->Size.X;
		float h = ch->Size.Y;
		Vector2 tl = ch->Bounds.Position;
		Vector2 br = ch->Bounds.Position + ch->Bounds.Size;

		/* Populate buffer. */
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x, y, tl.X, br.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x, y + h, tl.X, tl.Y);
		vertices[j++] = Vector4(x + w, y, br.X, br.Y);
		vertices[j++] = Vector4(x + w, y + h, br.X, tl.Y);

		pos.X += ch->Advance;
	}

	/* Create GPU side buffer. */
	vbo->Bind();
	vbo->SetData(vertices, size);
	free_s(vertices);
}

void Plutonium::FontRenderer::AddSingleString(Vector2 pos, const char * str)
{
	/* Make sure we check for the maximum string length. */
	LOG_THROW_IF(strlen(str) > MAX_STRING_LENGTH, "String '%s' if too long for the FontRenderer to handle!", str);

	strs.push_back(heapstr(str));
	vrtxs.push_back(pos);
}

void Plutonium::FontRenderer::WindowResizeEventHandler(WindowHandler sender, EventArgs args)
{
	/* Update projection matrix. */
	Rectangle viewport = sender->GetClientBounds();
	proj = Matrix::CreateOrtho(viewport.Position.X, viewport.Size.X, viewport.Position.Y, viewport.Size.Y, 0.0f, 1.0f);
}

void Plutonium::FontRenderer::ClearBuffer(void)
{
	/* Make sure we free the copies to the strings. */
	while (strs.size() > 0)
	{
		const char *cur = strs.back();
		free_cstr_s(cur);
		strs.pop_back();
	}

	vrtxs.clear();
}