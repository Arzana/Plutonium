#include "Graphics\Diagnostics\DebugSpriteRenderer.h"
#include "Graphics\Rendering\SpriteRenderer.h"
#include "Game.h"

Plutonium::DebugSpriteRenderer::DebugSpriteRenderer(Game * game, Vector2 resetPos, Vector2 moveMod)
	: DebugRenderer(game, resetPos, moveMod), spacing(10.0f)
{
	renderer = new SpriteRenderer(game->GetGraphics());
}

void Plutonium::DebugSpriteRenderer::AddDebugTexture(const Texture * sprite, Color color, Vector2 scale, float rotation)
{
	queue.push({ sprite, GetDrawPos(), color, scale, rotation });
	UpdateDrawPos(sprite->GetSize() * scale + spacing);
}

void Plutonium::DebugSpriteRenderer::Render(float)
{
	renderer->Begin();

	while (queue.size() > 0)
	{
		SpriteRenderArgs &cur = queue.front();
		renderer->Render(cur.sprite, cur.pos, cur.clr, cur.scale, cur.theta);
		queue.pop();
	}

	renderer->End();
	Reset();
}

void Plutonium::DebugSpriteRenderer::Finalize(void)
{
	DebugRenderer::Finalize();
	delete_s(renderer);
}