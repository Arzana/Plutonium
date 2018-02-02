#pragma once
#include "Graphics\Window.h"
#include "Input\Cursor.h"
#include "Input\Keyboard.h"
#include "GameComponent.h"

namespace Plutonium
{
	/* Defines a game loop object. */
	struct Game
	{
	public:
		/* Whether or not the update rate is fixed. */
		bool FixedTimeStep;

		/* Initializes a new instance of a game. */
		Game(_In_ const char *name);
		Game(_In_ const Game &value) = delete;
		Game(_In_ Game &&value) = delete;
		/* Releases the resources allocated by the game object. */
		~Game(void);

		_Check_return_ Game& operator =(_In_ const Game &other) = delete;
		_Check_return_ Game& operator =(_In_ Game &&other) = delete;

		/* Sets the target time step in Hz. */
		void SetTargetTimeStep(_In_ int value);
		/* Sets the current load percentage. */
		void SetLoadPercentage(_In_ int value);
		/* Adds a specified value to the load percentage. */
		void UpdateLoadPercentage(_In_ int value)
		{
			SetLoadPercentage(loadPercentage + value);
		}

		/* Starts the game and runs it until the game is closed. */
		void Run(void);
		/* Loads a new level for the game. */
		void LoadNew(void);

	protected:
		/* Gets the game window associated with this game. */
		_Check_return_ inline Window* GetWindow(void) const
		{
			return wnd;
		}

		/* Gets the cursor helper object for this game. */
		_Check_return_ inline Cursor* GetCursor(void) const
		{
			return cursor;
		}

		/* Gets the keyboard helper object for this game. */
		_Check_return_ inline KeyHandler GetKeyboard(void) const
		{
			return keyboard;
		}

		/* 
		Adds a component to the game,
		this component will be initialized after Initilaize is called but before LoadContent is called.
		These component will also not be reloaded when a new level is loaded!
		Note that the framework deallocates them with delete!
		*/
		void AddComponent(_In_ GameComponent *component);
		/* Exits the game. */
		void Exit(void);

		/* Initializes the global objects needed for the game to run. */
		virtual void Initialize(void) = 0;
		/* Initializes the level specific objects needed for the game to run. */
		virtual void LoadContent(void) = 0;
		/* Finalizes the level specific objects needed for the game to run. */
		virtual void UnLoadContent(void) = 0;
		/* Finalizes the global objects needed for the game to run. */
		virtual void Finalize(void) = 0;
		/* Updates the game. */
		virtual void Update(_In_ float dt) = 0;
		/* Renders the game. */
		virtual void Render(_In_ float dt) = 0;
		/* Renders the loading screen when the percentage is updated. */
		virtual void RenderLoad(_In_ float dt, _In_ int percentage) = 0;

	private:
		bool suppressRender;
		double prevTime;
		float targetElapTimeFocused;
		float targetElapTimeNoFocus;
		float accumElapTime;
		float maxElapTime;
		int loadPercentage;

		Window *wnd;
		Cursor *cursor;
		Keyboard *keyboard;
		std::vector<GameComponent*> components;

		bool Tick(void);
		void DoInitialize(void);
		void DoFinalize(void);
		void DoUpdate(float dt);
		void BeginRender(void);
		void DoRender(float dt);
		void EndRender(void);
	};
}