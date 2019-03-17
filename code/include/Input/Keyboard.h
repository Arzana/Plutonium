#pragma once
#include "InputDevice.h"
#include "Core/Events/EventBus.h"

namespace Pu
{
	/* Defines a helper object to receive keyboard data. */
	class Keyboard
		: public InputDevice
	{
	public:
		/* Occurs when a Unicode character is given by the user. */
		static EventBus<const Keyboard, wchar_t> CharInput;
		/* Occurs when a specific physical key is pressed. */
		EventBus<const Keyboard, uint16> KeyDown;
		/* Occurs when a specific physical key is released. */
		EventBus<const Keyboard, uint16> KeyUp;

	private:
		friend class InputDeviceHandler;

#ifdef _WIN32
		Keyboard(HANDLE hndl, const wstring &name, const RID_DEVICE_INFO &info);

		void HandleWin32Event(const RAWKEYBOARD &info);
#endif
	};
}