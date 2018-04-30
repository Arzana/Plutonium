#include "Graphics\Diagnostics\DebugTextRenderer.h"
#include "Graphics\Text\TextRenderer.h"
#include "Game.h"

Plutonium::DebugFontRenderer::DebugFontRenderer(Game * game, const char * font, const char * vrtxShdr, const char * fragShdr, int loadWeight, Vector2 resetPos, Vector2 moveMod)
	: DebugRenderer(game, resetPos, moveMod)
{
	renderer = new FontRenderer(game, font, vrtxShdr, fragShdr, loadWeight);
}

void Plutonium::DebugFontRenderer::AddDebugString(const char * str, Color clr)
{
	renderer->AddString(GetDrawPos(), str, clr);
	UpdateDrawPos(renderer->GetFont()->MeasureString(str) + Vector2(5.0f));
}

void Plutonium::DebugFontRenderer::Render(float dt)
{
	renderer->Render();
	Reset();
}

void Plutonium::DebugFontRenderer::Finalize(void)
{
	DebugRenderer::Finalize();
	delete_s(renderer);
}