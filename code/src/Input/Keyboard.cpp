#include "Input/Keyboard.h"
#include "Core/Diagnostics/Logging.h"

Pu::EventBus<const Pu::Keyboard, wchar_t> Pu::Keyboard::CharInput("KeyboardCharInput");

#ifdef _WIN32
Pu::Keyboard::Keyboard(HANDLE hndl, const wstring & name, const RID_DEVICE_INFO & info)
	: InputDevice(hndl, name, InputDeviceType::Keyboard, info),
	KeyDown("KeyboardKeyDown"), KeyUp("KeyboardKeyUp")
{}

void Pu::Keyboard::HandleWin32Event(const RAWKEYBOARD & info)
{
	if (info.Flags == RI_KEY_MAKE) KeyDown.Post(*this, info.VKey);
	else if (info.Flags == RI_KEY_BREAK)  KeyUp.Post(*this, info.VKey);
}
#endif