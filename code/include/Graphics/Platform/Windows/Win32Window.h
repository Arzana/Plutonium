#pragma once

#ifdef _WIN32
#include "Graphics/Platform/NativeWindow.h"
#include "Graphics/Vulkan/Instance.h"

namespace Pu
{
	/* Defines a window on the Windows platform. */
	class Win32Window
		: public NativeWindow
	{
	public:
		/* Defines whether the Alt-F4 close command is allowed. */
		bool AllowAltF4;

		/* Initializes a new instance of a Windows window (defaults to windowed mode). */
		Win32Window(_In_ VulkanInstance &vulkan, _In_ const wstring &title);
		Win32Window(_In_ const Win32Window&) = delete;
		Win32Window(_In_ Win32Window&&) = delete;
		/* Releases the resources allocated by the window. */
		~Win32Window(void);

		_Check_return_ Win32Window& operator =(_In_ const Win32Window&) = delete;
		_Check_return_ Win32Window& operator =(_In_ Win32Window&&) = delete;

		/* Gets whether the two native windows are equal. */
		_Check_return_ virtual bool operator ==(_In_ const NativeWindow &other) override;
		/* Gets whether the two native windows differ. */
		_Check_return_ virtual bool operator !=(_In_ const NativeWindow &other) override;

		/* Gets the displayed title of this window. */
		_Check_return_ virtual inline const wstring& GetTitle(void) const override
		{
			return title;
		}

		/* Gets the graphics bounds of the window. */
		_Check_return_ virtual inline const Viewport& GetClientBounds(void) const override
		{
			return vp;
		}

		/* Gets the current mode of the window. */
		_Check_return_ virtual inline WindowMode GetWindowMode(void) const override 
		{
			return mode;
		}

		/* Gets whether the window has focus. */
		_Check_return_ virtual inline bool HasFocus(void) const override
		{
			return focused;
		}

		/* Gets the physical location of the window. */
		_Check_return_ inline Vector2 GetPosition(void) const override
		{
			return pos;
		}

		/* Gets the default height of a Win32 window title bar. */
		_Check_return_ static int32 GetDefaultTitleBarHeight(void);

		/* Displays the window and gives it focus. */
		virtual void Show(void) override;
		/* Hides the window and revokes its focus. */
		virtual void Hide(void) override;
		/* Requests that the window be shut down after the next update call. */
		virtual void Close(void) override;
		/* Resizes the window to the specified size. */
		virtual void Resize(_In_ Vector2 newSize) override;
		/* Moves the window to the specified location. */
		virtual void Move(_In_ Vector2 newLocation) override;
		/* Sets the display mode of the window. */
		virtual void SetMode(_In_ WindowMode newMode) override;

	protected:
		/* Occurs when an input event is broadcasted. */
		EventBus<const Win32Window, const RAWINPUT&> OnInputEvent;
		/* Occurs when an input device is added. */
		EventBus<const Win32Window, HANDLE> InputDeviceAdded;
		/* Occurs when an input device is removed. */
		EventBus<const Win32Window, HANDLE> InputDeviceRemoved;
		/* Occurs when a unicode character is provided by the user. */
		EventBus<const Win32Window, wchar_t> OnCharInput;

		/* Updates the window, returns whether the window should remain active. */
		_Check_return_ virtual bool Update(void) override;

	private:
		friend class InputDeviceHandler;
		friend class Mouse;

		HINSTANCE instance;
		HWND hndl;
		const wstring title;
		Viewport vp;
		Vector2 pos;
		WindowMode mode;
		bool shouldClose, focused;
		RAWINPUT *input;
		uint32 rawInputSize;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void Move(int x, int y);
		void Resize(int w, int h);
		void UpdateClientArea(void);
		LRESULT HandleProc(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleSysCmd(WPARAM wParam, LPARAM lParam);
		LRESULT HandleInput(WPARAM wParam, LPARAM lParam);
	};
}
#endif