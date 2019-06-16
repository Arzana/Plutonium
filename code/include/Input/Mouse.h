#pragma once
#include "ButtonEventArgs.h"
#include "InputDevice.h"
#include "Graphics/Platform/NativeWindow.h"

namespace Pu
{
	/* Defines a helper object to receive cursor data. */
	class Mouse
		: public InputDevice
	{
	public:
		/* Occurs when the cursor is moved, gives the delta movement. */
		EventBus<const Mouse, Vector2> Moved;
		/* Occurs when the scroll wheel state changes, gives the delta movement. */
		EventBus<const Mouse, int16> Scrolled;

		/* The unique indentifier of the cursor. */
		uint64 ID;
		/* The number of unique buttons on the cursor. */
		size_t ButtonCount;
		/* The number of data pointes per second. */
		size_t SampleRate;

		/* Gets the absolute position of the on screen OS cursor. */
		_Check_return_ static Vector2 GetPosition(void);
		/* Gets the position of thge on screen OS cursor relative to the specified window. */
		_Check_return_ static Vector2 GetPosition(_In_ const NativeWindow &wnd);
		/* Gets whether the OS cursor is visible. */
		_Check_return_ static bool IsCursorVisible(void);
		/* Hides the OS cursor. */
		static void HideCursor(void);
		/* Show the OS cursor. */
		static void ShowCursor(void);
		/* Restricts the OS cursor to a specific window. */
		static void LockCursor(const NativeWindow &window);
		/* Disables the movement restriction of the OS cursor. */
		static void FreeCursor(void);

	private:
		friend class InputDeviceHandler;

		Vector2 oldPos;

		static const NativeWindow *lockedWnd;
		static ButtonInformation buttonInfo;

#ifdef _WIN32
		Mouse(HANDLE hndl, const wstring &name, const RID_DEVICE_INFO &info);

		static void ClipMouse(const NativeWindow &window, ValueChangedEventArgs<Vector2>);

		void HandleWin32Event(const RAWMOUSE &info);
#endif
	};
}