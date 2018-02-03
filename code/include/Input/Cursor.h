#pragma once
#include "Core\Math\Vector2.h"
#include "Core\Events\EventBus.h"
#include "ButtonEventArgs.h"

struct GLFWwindow;

namespace Plutonium
{
	struct Window;

	/* Defines a helper object for handling the cursor on a per window basis. */
	typedef const struct Cursor
	{
	public:
		/* The horizontal component of the cursor position. */
		int X;
		/* The vertical component of the cursor position. */
		int Y;
		/* The movement since the last frame of the scroll wheel or track pad. */
		Vector2 ScrollWheel;
		/* Whether the left button is down. */
		bool LeftButton;
		/* Whether the right button is down. */
		bool RightButton;
		/* Whether the middle button is down. */
		bool MiddleButton;

		/* Occures when the cursor enters the window bounds. */
		EventBus<Window, const Cursor*> EnterWindow;
		/* Occures when the cursor leaves the window bounds. */
		EventBus<Window, const Cursor*> LeaveWindow;
		/* Occurs when the state of a special button is changed. */
		EventBus<Window, const Cursor*, const ButtonEventArgs> SpecialButtonPress;

		/* Releases the resources allocated by the cursor. */
		~Cursor(void) noexcept;

		/* Gets the current position of the cursor. */
		_Check_return_ Vector2 GetPosition(void) const;
		/* Sets the cursor position, relative to the screen! */
		void SetPosition(_In_ Vector2 position);
		/* Disables the cursor, disabling it from moving and causing events. */
		void Disable(void);
		/* Hides the cursor, rending it invisible when it's over the associated window. */
		void Hide(void);
		/* Shows the curson in the normal mode. */
		void Show(void);

	private:
		friend struct Game;
		friend Cursor* GetCursorFromHndlr(GLFWwindow*);
		friend void GlfwCursorFocusEventHandler(GLFWwindow*, int);
		friend void GlfwCursorButtonEventHandler(GLFWwindow *, int, int, int);

		const Window *wnd;

		Cursor(const Window *wnd);

		void Update(void);
	} *CursorHandler;
}