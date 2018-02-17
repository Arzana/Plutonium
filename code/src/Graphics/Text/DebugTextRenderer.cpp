#include "Graphics\Text\DebugTextRenderer.h"
#include "Core\StringFunctions.h"

Plutonium::DebugFontRenderer::DebugFontRenderer(WindowHandler wnd, const char * font, const char * vrtxShdr, const char * fragShdr)
	: FontRenderer(wnd, font, vrtxShdr, fragShdr)
{}

void Plutonium::DebugFontRenderer::AddDebugString(const char * str)
{
	AddString(defPos, str);
	defPos.Y -= font->GetLineSpace() * (1 + cntchar(str, '\n'));
}

void Plutonium::DebugFontRenderer::Render(void)
{
	/* Render actual text. */
	FontRenderer::Render();

	/* Reset frame draw position. */
	defPos = Vector2(0.0f, wnd->GetClientBounds().GetHeight() - font->GetLineSpace());
}