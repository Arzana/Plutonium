#include "Graphics\Diagnostics\DebugTextRenderer.h"
#include "Core\StringFunctions.h"

Plutonium::DebugFontRenderer::DebugFontRenderer(GraphicsAdapter * device, const char * font, const char * vrtxShdr, const char * fragShdr, Vector2 resetPos)
	: FontRenderer(device, font, vrtxShdr, fragShdr), reset(resetPos), defPos(resetPos)
{}

void Plutonium::DebugFontRenderer::AddDebugString(const char * str, Color clr)
{
	AddString(defPos, str, clr);
	defPos.Y += font->GetLineSpace() * (1 + cntchar(str, '\n'));
}

void Plutonium::DebugFontRenderer::Render(void)
{
	/* Render actual text. */
	FontRenderer::Render();

	/* Reset frame draw position. */
	defPos = reset;
}