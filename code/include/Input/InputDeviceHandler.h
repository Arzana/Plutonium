#pragma once
#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"
#include "Graphics/Platform/NativeWindow.h"

namespace Pu
{
	class Win32Window;

	/* Creates and manages all input devices. */
	class InputDeviceHandler
	{
	public:
		/* Occurs when any key on any input device is pressed. */
		EventBus<const InputDevice, const ButtonEventArgs&> AnyKeyDown;
		/* Occurs when any key on any input device is released. */
		EventBus<const InputDevice, const ButtonEventArgs&> AnyKeyUp;
		/* Occurs when any slider value is changed. */
		EventBus<const InputDevice, const ValueEventArgs&> AnyValueChanged;
		/* Occurs when a mouse is scrolled. */
		EventBus<const Mouse, int16> AnyMouseScrolled;
		/* Occurs when a mouse is moved. */
		EventBus<const Mouse, Vector2> AnyMouseMoved;

		InputDeviceHandler(_In_ const InputDeviceHandler&) = delete;
		InputDeviceHandler(_In_ InputDeviceHandler&&) = delete;

		_Check_return_ InputDeviceHandler& operator =(_In_ const InputDeviceHandler&) = delete;
		_Check_return_ InputDeviceHandler& operator =(_In_ InputDeviceHandler&&) = delete;

		/* Gets the amount of other human interface devices currently available. */
		_Check_return_ inline size_t GetHIDCount(void) const
		{
			return hids.size();
		}

		/* Gets the human interface device at the specific index. */
		_Check_return_ inline InputDevice& GetHID(_In_ size_t idx)
		{
			return hids.at(idx);
		}

		/* Gets the amount of cursors currently available. */
		_Check_return_ inline size_t GetCursorCount(void) const
		{
			return mouses.size();
		}

		/* Gets the cursor at the specific index. */
		_Check_return_ inline Mouse& GetCursor(_In_ size_t idx)
		{
			return mouses.at(idx);
		}

		/* Gets the amount of keyboards currently available. */
		_Check_return_ inline size_t GetKeyboardCount(void) const
		{
			return keyboards.size();
		}

		/* Gets the keyboard at the specific index. */
		_Check_return_ inline Keyboard& GetKeyboard(_In_ size_t idx)
		{
			return keyboards.at(idx);
		}

		/* Gets the amount of game pads currently available. */
		_Check_return_ inline size_t GetGamePadCount(void) const
		{
			return gamepads.size();
		}

		/* Gets the game pad at the specific index. */
		_Check_return_ inline GamePad& GetGamePad(_In_ size_t idx)
		{
			return gamepads.at(idx);
		}

	private:
		friend class Application;

		vector<Mouse> mouses;
		vector<Keyboard> keyboards;
		vector<GamePad> gamepads;
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