#include "Graphics\Text\DebugTextRenderer.h"
#include "Core\StringFunctions.h"

Plutonium::DebugFontRenderer::DebugFontRenderer(GraphicsAdapter *device, const char * font, const char * vrtxShdr, const char * fragShdr)
	: FontRenderer(device, font, vrtxShdr, fragShdr)
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
	defPos = Vector2(0.0f, device->GetWindow()->GetClientBounds().GetHeight() - font->GetLineSpace());
}