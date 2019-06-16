#include "Input/Keyboard.h"
#include "Core/Diagnostics/Logging.h"

Pu::ButtonInformation Pu::Keyboard::buttonInfo = Pu::ButtonInformation(Pu::HIDUsageGenericDesktop::Keyboard);

Pu::EventBus<const Pu::Keyboard, wchar_t> Pu::Keyboard::CharInput("KeyboardCharInput");

#ifdef _WIN32
Pu::Keyboard::Keyboard(HANDLE hndl, const wstring & name, const RID_DEVICE_INFO & info)
	: InputDevice(hndl, name, InputDeviceType::Keyboard, info)
{}

void Pu::Keyboard::HandleWin32Event(const RAWKEYBOARD & info)
{
	/*
	There are two other flags E0 and E1 these are used to denote special keys,
	for our purposes we can simply ignore these and just check whether the key is down or up.
	*/
	const ButtonEventArgs args(buttonInfo, info.VKey);

	if (info.Flags & RI_KEY_BREAK) KeyUp.Post(*this, args);
	else KeyDown.Post(*this, args);
}
#endif