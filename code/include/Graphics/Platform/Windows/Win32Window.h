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

		/* Initializes a new instance of a Windows window. */
		Win32Window(_In_ VulkanInstance &vulkan, _In_ const char *title, _In_ Vector2 size);
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
		_Check_return_ virtual inline const char* GetTitle(void) const override
		{
			return title;
		}

		/* Gets the graphics bounds of the window. */
		_Check_return_ virtual const Rectangle& GetClientBounds(void) const override
		{
			return vp;
		}

		/* Gets the current mode of the window. */
		_Check_return_ virtual WindowMode GetWindowMode(void) const override 
		{
			return mode;
		}

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
		/* Gets the surface to render to. */
		_Check_return_ virtual inline Surface& GetSurface(void) override
		{
			return *surface;
		}

		/* Updates the window, returns whether the window should remain active. */
		_Check_return_ virtual bool Update(void) override;

	private:
		Surface *surface;
		HINSTANCE instance;
		HWND hndl;
		const char *title;
		Rectangle vp;
		WindowMode mode;
		bool shouldClose, focused;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		void Move(int x, int y);
		void Resize(int w, int h);
		void UpdateClientArea(void);
		LRESULT HandleProc(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleSysCmd(WPARAM wParam, LPARAM lParam);
	};
}
#endif