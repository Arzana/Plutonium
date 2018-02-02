#pragma once
#include "Core\Events\EventBus.h"
#include "KeyEventArgs.h"

struct GLFWwindow;

namespace Plutonium
{
	struct Window;

	/* Defines a helper object for the keyboard on a per window basis. */
	typedef const struct Keyboard
	{
	public:
		/* Occurs when a key event is raised. */
		EventBus<Window, const KeyEventArgs> KeyPress;
		/* Occurs when a UTF-32 character with modifiers is specified. */
		EventBus<Window, uint32> CharInput;

		/* Releases the resources allocated by the keyboard. */
		~Keyboard(void);

		/* Checks whether a specified key is pressed or repeated. */
		_Check_return_ bool IsKeyDown(_In_ Keys key) const;
		/* Checks whether a specified key is released. */
		_Check_return_ bool IsKeyUp(_In_ Keys key) const;
		/* Gets the current state of a specified key. */
		_Check_return_ KeyState GetKey(_In_ Keys key) const;

	private:
		friend struct Game;
		friend Keyboard* GetKeyboardFromHndlr(GLFWwindow*);
		friend void GlfwKeyInputEventHandler(GLFWwindow*, int, int, int, int);
		friend void GlfwCharInputEventHandler(GLFWwindow*, uint32);

		const Window *wnd;

		Keyboard(const Window *wnd);
	} *KeyHandler;
}