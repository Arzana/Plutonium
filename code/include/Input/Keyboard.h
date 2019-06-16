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

	private:
		friend class InputDeviceHandler;

		static ButtonInformation buttonInfo;

#ifdef _WIN32
		Keyboard(HANDLE hndl, const wstring &name, const RID_DEVICE_INFO &info);

		void HandleWin32Event(const RAWKEYBOARD &info);
#endif
	};
}