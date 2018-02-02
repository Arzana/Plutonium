#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"

namespace Plutonium
{
	struct Game;

	/* Defines a basic game component that needs to be updated. */
	struct GameComponent
	{
	public:
		/* Occurs when the state of the game component is changed. */
		EventBus<GameComponent, EventArgs> StateChanged;

		/* Creates a new game component for a specified game. */
		GameComponent(_In_ Game *game);

		/* Enables the game component. */
		void Enable(void);
		/* Disables the game component. */
		void Disable(void);
		/* 
		Sets the place in the update order. 
		-1 can be used to define that the place doesn't matter.
		*/
		void SetUpdatePlace(_In_ int place);

	protected:
		/* The game associated with this component. */
		Game *game;

		/* Initializes the game component. */
		virtual void Initialize(void);
		/* Updates the game component. */
		virtual void Update(float dt) = 0;
		/* Finalizes the game component. */
		virtual void Finalize(void);

	private:
		friend struct Game;
		friend bool ComponentSortPredicate(GameComponent *, GameComponent*);

		bool initialized;
		bool enabled;
		int place;
	};
}