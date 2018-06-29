#include "Graphics\Text\FontRenderer.h"
#include "Core\StringFunctions.h"

constexpr Plutonium::int32 MAX_STRING_LENGTH = 64;

Plutonium::FontRenderer::FontRenderer(Game *game, const char * font, const char * vrtxShdr, const char * fragShdr, int loadWeight)
	: parent(game), percentage(loadWeight), font(nullptr)
{
	WindowHandler wnd = game->GetGraphics()->GetWindow();

	/* Load shader and fields from files. */
	shdr = Shader::FromFile(vrtxShdr, fragShdr);
	clr = shdr->GetUniform("u_color");
	tex = shdr->GetUniform("u_texture");
	wvp = shdr->GetUniform("u_wvp");
	pos = shdr->GetAttribute("a_position");

	/* Load font from file. */
	game->GetLoader()->LoadFont(font, Callback<Font>(this, &FontRenderer::OnLoadComplete), 24.0f, true);

	/* Make sure projection matrix is updated on window resize. */
	WindowResizeEventHandler(wnd, EventArgs());
	wnd->SizeChanged.Add(this, &FontRenderer::WindowResizeEventHandler);

	vbo = new Buffer(wnd, BindTarget::Array);
	vbo->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, MAX_STRING_LENGTH * 6);
}

Plutonium::FontRenderer::~FontRenderer(void)
{
	/* Remove event handler. */
	parent->GetGraphics()->GetWindow()->SizeChanged.Remove(this, &FontRenderer::WindowResizeEventHandler);

	/* Delete shader and font. */
	delete_s(vbo);
	delete_s(shdr);
	parent->GetLoader()->Unload(font);
}

void Plutonium::FontRenderer::AddString(Vector2 pos, const char * str, Color clr)
{
	/* Make sure we don't allow string to be rendered when the font is not yet loaded. */
	if (!font) return;

	/* Initialize newline split buffer. */
	const size_t buffLen = cntchar(str, '\n') + 1;
	char **buffer = mallocaa_s(char, buffLen, FILENAME_MAX);

	/* Split string to lines and loop through them. */
	size_t len = spltstr(str, '\n', buffer, 0);
	ASSERT_IF(buffLen != len, "Something went wrong whilst splitting the string, this should never occur!");
	for (size_t i = 0; i < len; i++)
	{
		AddSingleString(parent->GetGraphics()->ToOpenGL(Vector2(pos.X, pos.Y + font->lineSpace * (i + 1))), buffer[i], clr);
	}

	freeaa_s(buffer, buffLen);
}

void Plutonium::FontRenderer::AddString(Vector2 pos, const char32 * str, Color clr)
{
	/* Make sure we don't allow string to be rendered when the font is not yet loaded. */
	if (!font) return;

	/* Initialize newline split buffer. */
	const size_t buffLen = cntchar(str, '\n') + 1;
	char32 **buffer = mallocaa_s(char32, buffLen, FILENAME_MAX);

	/* Split string to lines and loop through them. */
	size_t len = spltstr(str, '\n', buffer, 0);
	ASSERT_IF(buffLen != len, "Something went wrong whilst splitting the string, this should never occur!");
	for (size_t i = 0; i < len; i++)
	{
		AddSingleString(parent->GetGraphics()->ToOpenGL(Vector2(pos.X, pos.Y + font->lineSpace * (i + 1))), buffer[i], clr);
	}

	freeaa_s(buffer, buffLen);
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
		parent->GetGraphics()->SetAlphaSourceBlend(BlendType::ISrcAlpha);

		/* Render all stored strings. */
		for (size_t i = 0; i < strs.size(); i++)
		{
			RenderString(strs.at(i));
		}

		/* End shader and clear buffers. */
		shdr->End();
		ClearBuffer();
	}
}

void Plutonium::FontRenderer::RenderString(LineInfo *info)
{
	/* Update buffer. */
	UpdateVBO(info->Position, info->String);

	/* Set color and position parameters. */
	this->clr->Set(info->Color);
	this->pos->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

	/* Render line. */
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vbo->GetElementCount()));
}

void Plutonium::FontRenderer::UpdateVBO(Vector2 pos, const char32 *str)
{
	/* Create CPU side vertices buffer. (6 vertices per quad of vector4 type per glyph). */
	size_t size = strlen(str) * 6;
	Vector4 *vertices = malloca_s(Vector4, size);

	for (size_t i = 0, j = 0; i < (size / 6); i++)
	{
		/* Get current character. */
		const Character *ch = font->GetCharOrDefault(str[i]);

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
	vbo->SetData(vertices, size);
	freea_s(vertices);
}

void Plutonium::FontRenderer::AddSingleString(Vector2 pos, const char * str, Color clr)
{
	/* Make sure we check for the maximum string length. */
	LOG_THROW_IF(strlen(str) > MAX_STRING_LENGTH, "String '%s' if too long for the FontRenderer to handle!", str);
	strs.push_back(new LineInfo(str, pos, clr));
}

void Plutonium::FontRenderer::AddSingleString(Vector2 pos, const char32 * str, Color clr)
{
	/* Make sure we check for the maximum string length. */
	LOG_THROW_IF(strlen(str) > MAX_STRING_LENGTH, "String '%s' if too long for the FontRenderer to handle!", str);
	strs.push_back(new LineInfo(str, pos, clr));
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
	for (size_t i = 0; i < strs.size(); i++) delete_s(strs.at(i));
	strs.clear();
}

void Plutonium::FontRenderer::OnLoadComplete(const AssetLoader *, Font * result)
{
	font = result;
	parent->UpdateLoadPercentage(percentage);
}

Plutonium::FontRenderer::LineInfo::LineInfo(const char * string, Vector2 pos, Plutonium::Color clr)
	: String(heapwstr(string)), Position(pos), Color(clr)
{}

Plutonium::FontRenderer::LineInfo::LineInfo(const char32 * string, Vector2 pos, Plutonium::Color clr)
	: String(heapwstr(string)), Position(pos), Color(clr)
{}

Plutonium::FontRenderer::LineInfo::~LineInfo(void) noexcept
{
	free_s(String);
}
