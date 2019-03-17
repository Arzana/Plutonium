#include "Input/InputDevice.h"

#ifdef _WIN32
Pu::InputDevice::InputDevice(HANDLE hndl, const wstring &name, InputDeviceType type, const RID_DEVICE_INFO &info)
	: Name(name), Type(type), Info(info), Hndl(hndl)
{}
#endif