#pragma once
#include "Core/Math/Rectangle.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/ValueChangedEventArgs.h"
#include "WindowMode.h"

namespace Pu
{
	class Surface;

	/* Defines a base object for all platform-specific windows. */
	class NativeWindow
	{
	public:
		/* Occurs when the size of the window changed. */
		EventBus<const NativeWindow, ValueChangedEventArgs<Vector2>> OnSizeChanged;
		/* Occurs when the location of the window changed. */
		EventBus<const NativeWindow, ValueChangedEventArgs<Vector2>> OnLocationChanged;
		/* Occurs when the window gained focus. */
		EventBus<NativeWindow, EventArgs> OnGainedFocus;
		/* Occurs when the window loses focus. */
		EventBus<NativeWindow, EventArgs> OnLostFocus;

		NativeWindow(_In_ const NativeWindow&) = delete;
		NativeWindow(_In_ const NativeWindow&&) = delete;
		/* Releases the resources allocated by the native window. */
		virtual ~NativeWindow(void) {}

		_Check_return_ NativeWindow& operator =(_In_ const NativeWindow&) = delete;
		_Check_return_ NativeWindow& operator =(_In_ NativeWindow&&) = delete;

		/* Gets whether the two native windows are equal. */
		_Check_return_ virtual bool operator ==(_In_ const NativeWindow &other) = 0;
		/* Gets whether the two native windows differ. */
		_Check_return_ virtual bool operator !=(_In_ const NativeWindow &other) = 0;

		/* Gets the displayed title of this window. */
		_Check_return_ virtual const char* GetTitle(void) const = 0;
		/* Gets the graphics bounds of the window. */
		_Check_return_ virtual const Rectangle& GetClientBounds(void) const = 0;
		/* Gets the current mode of the window. */
		_Check_return_ virtual WindowMode GetWindowMode(void) const = 0;

		/* Gets the aspect ratio of the window. */
		_Check_return_ inline bool GetAspectRatio(void) const
		{
			const Rectangle &vp = GetClientBounds();
			return vp.GetWidth() / vp.GetHeight();
		}

		/* Displays the window and gives it focus. */
		virtual void Show(void) = 0;
		/* Hides the window and revokes its focus. */
		virtual void Hide(void) = 0;
		/* Requests that the window be shut down after the next update call. */
		virtual void Close(void) = 0;
		/* Resizes the window to the specified size. */
		virtual void Resize(_In_ Vector2 newSize) = 0;
		/* Moves the window to the specified location. */
		virtual void Move(_In_ Vector2 newLocation) = 0;
		/* Sets the display mode of the window. */
		virtual void SetMode(_In_ WindowMode mode) = 0;

		inline bool testUpdate(void)
		{
			return Update();
		}

	protected:
		/* Initializes the global instance of a native window. */
		NativeWindow(void);

		/* Gets the surface to render to. */
		_Check_return_ virtual Surface& GetSurface(void) = 0;
		/* Updates the window, returns whether the window should remain active. */
		_Check_return_ virtual bool Update(void) = 0;
	};
}