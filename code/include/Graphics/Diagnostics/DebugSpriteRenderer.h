#pragma once
#include "Graphics\Rendering\SpriteRenderer.h"

namespace Plutonium
{
	struct DebugSpriteRenderer
		: SpriteRenderer
	{
	public:
		/* Initializes a new instance of a debug font renderer. */
		DebugSpriteRenderer(_In_ GraphicsAdapter *device, _In_ const char *vrtxShdr, _In_ const char *fragShdr, _In_opt_ Vector2 resetPos = Vector2::Zero);
		DebugSpriteRenderer(_In_ const DebugSpriteRenderer &value) = delete;
		DebugSpriteRenderer(_In_ DebugSpriteRenderer &&value) = delete;

		_Check_return_ DebugSpriteRenderer& operator =(_In_ const DebugSpriteRenderer &other) = delete;
		_Check_return_ DebugSpriteRenderer& operator =(_In_ DebugSpriteRenderer &&other) = delete;

		/* Renders a debug sprite at the debug sprite position. */
		void RenderDebug(_In_ const Texture *sprite, _In_opt_ Color color = Color::White, _In_opt_ Vector2 scale = Vector2::One, _In_opt_ float rotation = 0.0f);
		/* Stops rendering the specified scene. */
		void End(void);

	private:
		const float spacing;
		const Vector2 reset;
		Vector2 defPos;
	};
}