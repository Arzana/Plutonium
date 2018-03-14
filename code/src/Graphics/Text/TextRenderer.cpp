#include "Graphics\Text\TextRenderer.h"
#include "Core\StringFunctions.h"

Plutonium::FontRenderer::FontRenderer(WindowHandler wnd, const char * font, const char * vrtxShdr, const char * fragShdr)
	: wnd(wnd)
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
	WindowResizeEventHandler(wnd, EventArgs());
	wnd->SizeChanged.Add(this, &FontRenderer::WindowResizeEventHandler);

	/* Create buffers for the character vertices. */
	vbo = new Buffer();
	vbo->Bind(BindTarget::Array);
	vbo->SetData<Vector4>(BufferUsage::DynamicDraw, nullptr, 6);

#if defined(DEBUG)
	LOG_WAR("FontRenderer is leaking memory on debug mode!");
#endif
}

Plutonium::FontRenderer::~FontRenderer(void)
{
	/* Remove event handler. */
	wnd->SizeChanged.Remove(this, &FontRenderer::WindowResizeEventHandler);

	/* Delete shader and font. */
	delete_s(shdr);
	delete_s(font);
	delete_s(vbo);
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

		/* Make sure blending is enabled. */
		EnableBlending();

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
	/* Set color for string. */
	this->clr->Set(clr);

	/* Loop through string. */
	char c = *string;
	for (size_t i = 0; c != '\0'; i++, c = string[i])
	{
		/* Get current character and set texture. */
		Character *ch = font->GetCharOrDefault(c);

		/* For character that don't define a texture (space, etc.) we skip the render stage. */
		if (ch->Texture)
		{
			tex->Set(ch->Texture);

			/* Defines components of the position vertices. */
			float x = pos.X + ch->Bearing.X;
			float y = pos.Y - (ch->Size.Y + ch->Bearing.Y);
			float w = ch->Size.X;
			float h = ch->Size.Y;

			/* Define vertices. */
			float vertices[6][4] =
			{
				{ x, y + h, 0.0f, 0.0f },
				{ x, y, 0.0f, 1.0f },
				{ x + w, y, 1.0f, 1.0f },
				{ x, y + h, 0.0f, 0.0f },
				{ x + w, y, 1.0f, 1.0f },
				{ x + w, y + h, 1.0f, 0.0f }
			};

			/* Bind buffers and draw character. */
			vbo->Bind();
			vbo->SetData(vertices);
			this->pos->Initialize(false, sizeof(Vector4), offset_ptr(Vector4, X));

			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vbo->GetElementCount()));
		}

		/* Update draw position. */
		pos.X += ch->Advance;
	}
}

void Plutonium::FontRenderer::AddSingleString(Vector2 pos, const char * str)
{
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

void Plutonium::FontRenderer::EnableBlending(void)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}