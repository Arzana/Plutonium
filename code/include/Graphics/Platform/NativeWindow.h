#pragma once
#include "Core/Math/Vector2.h"
#include "Core/Events/EventBus.h"
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Core/Events/ValueChangedEventArgs.h"
#include "WindowMode.h"
#include "Display.h"

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
		EventBus<NativeWindow> OnGainedFocus;
		/* Occurs when the window loses focus. */
		EventBus<NativeWindow> OnLostFocus;

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
		_Check_return_ virtual const wstring& GetTitle(void) const = 0;
		/* Gets the graphics bounds of the window. */
		_Check_return_ virtual const Viewport& GetClientBounds(void) const = 0;
		/* Gets the position of the window relative the monitor. */
		_Check_return_ virtual Vector2 GetPosition(void) const = 0;
		/* Gets whether the window has focus. */
		_Check_return_ virtual bool HasFocus(void) const = 0;

		/* Gets the current mode of the window. */
		_Check_return_ inline WindowMode GetWindowMode(void) const
		{
			return mode;
		}

		/* Gets the size of the window. */
		_Check_return_ inline Extent2D GetSize(void) const
		{
			return GetClientBounds().GetSize();
		}

		/* Gets the offset of the window to the origin of the surface. */
		_Check_return_ inline Offset2D GetOffset(void) const
		{
			const Vector2 pos = GetPosition();
			return Offset2D(static_cast<int32>(pos.X), static_cast<int32>(pos.Y));
		}

		/* Gets the aspect ratio of the window. */
		_Check_return_ inline float GetAspectRatio(void) const
		{
			const Viewport &vp = GetClientBounds();
			return vp.Width / vp.Height;
		}

		/* Gets the display that houses the window. */
		_Check_return_ inline const Display& GetDisplay(void) const
		{
			return Display::GetDisplayAt(GetClientBounds().GetPosition());
		}

		/* Gets the surface associated with this window. */
		_Check_return_ inline const Surface& GetSurface(void) const
		{
			return *surface;
		}

		/* Resizes the window to the specified size. */
		inline void Resize(_In_ float width, _In_ float height)
		{
			Resize(Vector2(width, height));
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

	protected:
		friend class GameWindow;
		friend class Application;

		/* Defines whether the next render call should be suppressed. */
		bool shouldSuppressRender;
		/* Defines the surface associated with this window (NativeWindow doesn't take ownership!). */
		Surface *surface;
		/* Defines the current mode of the window. */
		WindowMode mode;

		/* Initializes the global instance of a native window. */
		NativeWindow(void);

		/* Updates the window, returns whether the window should remain active. */
		_Check_return_ virtual bool Update(void) = 0;
	};
}