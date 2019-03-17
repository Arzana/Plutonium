#pragma once
#include "Cursor.h"
#include "Keyboard.h"
#include "Graphics/Platform/NativeWindow.h"

namespace Pu
{
	class Win32Window;

	/* Creates and manages all input devices. */
	class InputDeviceHandler
	{
	public:
		/* Defines a special cursor that hooks into all cursors used by the host. */
		Cursor AnyCursor;
		/* Defines a special keyboard that hooks into all keyboards used by the host. */
		Keyboard AnyKeyboard;

		InputDeviceHandler(_In_ const InputDeviceHandler&) = delete;
		InputDeviceHandler(_In_ InputDeviceHandler&&) = delete;

		_Check_return_ InputDeviceHandler& operator =(_In_ const InputDeviceHandler&) = delete;
		_Check_return_ InputDeviceHandler& operator =(_In_ InputDeviceHandler&&) = delete;

		/* Gets the amount of cursors currently available. */
		_Check_return_ inline size_t GetCursorCount(void) const
		{
			return cursors.size();
		}

		/* Gets the cursor at the specific index. */
		_Check_return_ inline Cursor& GetCursor(_In_ size_t idx)
		{
			return cursors.at(idx);
		}

		/* Gets the amount of keybaords currently available. */
		_Check_return_ inline size_t GetKeyboardCount(void) const
		{
			return keyboards.size();
		}

		/* Gets the keyboard at the specific index. */
		_Check_return_ inline Keyboard& GetKeyboard(_In_ size_t idx)
		{
			return keyboards.at(idx);
		}

	private:
		friend class Application;

		vector<Cursor> cursors;
		vector<Keyboard> keyboards;
		vector<InputDevice> hids;

		InputDeviceHandler(void);

#ifdef _WIN32
		inline void HandleWin32InputDeviceAdded(const Win32Window&, HANDLE hndl)
		{
			AddWin32InputDevice(hndl);
		}

		void RegisterInputDevicesWin32(const Win32Window &wnd) const;
		void HandleWin32InputEvent(const Win32Window&, const RAWINPUT &input);
		void HandleWin32InputDeviceRemoved(const Win32Window&, HANDLE hndl);
		void AddWin32InputDevice(HANDLE hndl);
#endif
	};
}