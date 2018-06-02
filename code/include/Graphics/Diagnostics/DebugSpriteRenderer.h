#pragma once
#include <queue>
#include "Graphics\Color.h"
#include "Graphics\Texture.h"
#include "Graphics\Diagnostics\DebugRenderer.h"

namespace Plutonium
{
	struct SpriteRenderer;

	/* Provides ease of use debug sprite functionality. */
	struct DebugSpriteRenderer
		: DebugRenderer
	{
	public:
		/* Initializes a new instance of a debug sprite renderer. */
		DebugSpriteRenderer(_In_ Game *game, _In_opt_ Vector2 resetPos = Vector2(0.0f, 100.0f), _In_opt_ Vector2 moveMod = Vector2::UnitY);
		DebugSpriteRenderer(_In_ const DebugSpriteRenderer &value) = delete;
		DebugSpriteRenderer(_In_ DebugSpriteRenderer &&value) = delete;

		_Check_return_ DebugSpriteRenderer& operator =(_In_ const DebugSpriteRenderer &other) = delete;
		_Check_return_ DebugSpriteRenderer& operator =(_In_ DebugSpriteRenderer &&other) = delete;

		/* Renders a debug sprite at the debug sprite position. */
		void AddDebugTexture(_In_ const Texture *sprite, _In_opt_ Color color = Color::White, _In_opt_ Vector2 scale = Vector2::One, _In_opt_ float rotation = 0.0f);

	protected:
		SpriteRenderer * renderer;

		virtual void Render(_In_ float dt) override;
		virtual void Finalize(void) override;

	private:
		struct SpriteRenderArgs
		{
			const Texture *sprite;
			Vector2 pos;
			Color clr;
			Vector2 scale;
			float theta;
		};

		const Vector2 spacing;
		std::queue<SpriteRenderArgs> queue;
	};
}