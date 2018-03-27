#include "Graphics\Diagnostics\DebugSpriteRenderer.h"

Plutonium::DebugSpriteRenderer::DebugSpriteRenderer(GraphicsAdapter * device, const char * vrtxShdr, const char * fragShdr, Vector2 resetPos)
	: SpriteRenderer(device, vrtxShdr, fragShdr), reset(resetPos), defPos(resetPos), spacing(10.0f)
{}

void Plutonium::DebugSpriteRenderer::RenderDebug(const Texture * sprite, Color color, Vector2 scale, float rotation)
{
	Render(sprite, defPos, color, scale, rotation);
	defPos.Y += (sprite->Height * scale.Y) + spacing;
}

void Plutonium::DebugSpriteRenderer::End(void)
{
	/* End actual renderer. */
	SpriteRenderer::End();

	/* Reset frame draw position. */
	defPos = reset;
}