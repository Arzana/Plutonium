#pragma once
#include "Core\Math\Vector2.h"
#include "Components\GameComponent.h"

namespace Plutonium
{
	/* Defines a base object for debug renderers. */
	class DebugRenderer
		: public GameComponent
	{
	public:
		/* Initializes a new instance of a debug renderer base. */
		DebugRenderer(_In_ Game *game, _In_ Vector2 resetPos, _In_ Vector2 moveMod)
			: GameComponent(game), reset(resetPos), mod(moveMod), pos(resetPos)
		{}

		DebugRenderer(_In_ const DebugRenderer &value) = delete;
		DebugRenderer(_In_ DebugRenderer &&value) = delete;

		_Check_return_ DebugRenderer& operator =(_In_ const DebugRenderer &other) = delete;
		_Check_return_ DebugRenderer& operator =(_In_ DebugRenderer &&other) = delete;

	protected:
		/* Renders the debug renderer. */
		virtual void Render(_In_ float dt) = 0;

		/* Get the position at which the current debug object should be drawn. */
		_Check_return_ inline Vector2 GetDrawPos(void) const
		{
			return pos;
		}

		/* Updates the draw position with the size of the current object. */
		inline void UpdateDrawPos(_In_ Vector2 size)
		{
			pos += size * mod;
		}

		/* Resets the debug renderer. */
		inline void Reset(void)
		{
			pos = reset;
		}

	private:
		const Vector2 reset, mod;
		Vector2 pos;
	};
}