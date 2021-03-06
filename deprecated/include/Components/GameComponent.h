#pragma once
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"

namespace Plutonium
{
	class Game;

	/* Defines a basic game component that needs to be updated and can be rendered. */
	class GameComponent
	{
	public:
		/* Indicates that this component should be actively updated (and drawn if desired) during load time. */
		bool ActiveDuringLoad;

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
		virtual void Update(_In_ float dt);
		/* Renders the game component. */
		virtual void Render(_In_ float dt);
		/* Finalizes the game component. */
		virtual void Finalize(void);

	private:
		friend class Game;
		friend bool ComponentSortPredicate(GameComponent *, GameComponent*);

		bool initialized;
		bool enabled;
		int place;
	};
}