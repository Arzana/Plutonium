#pragma once
#include "Core\Math\Rectangle.h"
#include "Core\Events\EventBus.h"
#include "Core\Events\EventArgs.h"
#include "WindowModes.h"

struct GLFWwindow;

namespace Plutonium
{
	/* Defines a window handler. */
	typedef const struct Window
	{
	public:
		/* Occures when the window is resized. */
		EventBus<Window, EventArgs> SizeChanged;
		/* Occures when the window gains focus. */
		EventBus<Window, EventArgs> GainedFocus;
		/* Occures when the window loses focus. */
		EventBus<Window, EventArgs> LostFocus;

		/* Initializes a new instance of a window with a specified title and size. */
		Window(_In_ const char *title, _In_ Vector2 size);
		Window(_In_ const Window &value) = delete;
		Window(_In_ Window &&value) = delete;
		/* Releases the resources allocated by the window. */
		~Window(void);

		_Check_return_ Window& operator =(_In_ const Window &other) = delete;
		_Check_return_ Window& operator =(_In_ Window &&other) = delete;

		/* Gets the display title of the window. */
		_Check_return_ inline const char* GetTitle(void) const
		{
			return title;
		}

		/* Gets the viewport bounds of the window. */
		_Check_return_ inline const Rectangle& GetClientBounds(void) const
		{
			return vpBounds;
		}

		/* Gets the bounds of the window. */
		_Check_return_ inline const Rectangle& GetWinowBounds(void) const
		{
			return wndBounds;
		}

		/* Calculates the aspect ration of the windows viewport. */
		_Check_return_ inline float AspectRatio(void) const
		{
			return vpBounds.GetWidth() / vpBounds.GetHeight();
		}

		/* Gets whether the window is currently focused. */
		_Check_return_ inline bool HasFocus(void) const
		{
			return focused;
		}

		/* Displayes the window and gives it focus. */
		void Show(void) const;
		/* Hides the window and revokes its focus. */
		void Hide(void) const;
		/* Requests that the window be shut down after the next update call. */
		void Close(void) const;
		/* Resizes the window to a specified size. */
		void Resize(_In_ Vector2 size);
		/* Moves the window to a new position. */
		void Move(_In_ Vector2 position);
		/* Alters the display mode of the window. */
		void SetMode(_In_ WindowMode mode);

	private:
		friend struct Game;
		friend struct Keyboard;
		friend struct Cursor;
		friend Window* GetWndFromHndlr(GLFWwindow*);
		friend void GlfwFocusChangedEventHandler(GLFWwindow*, int);

		bool Update(void);
		void SetBounds(Vector2 pos, Vector2 size);

		const char *title;
		Rectangle wndBounds, vpBounds;
		GLFWwindow *hndlr;
		bool operational;
		WindowMode mode;
		bool allowResizing;
		mutable bool focused;

	} *WindowHandler;
}