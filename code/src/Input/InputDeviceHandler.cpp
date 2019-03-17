#include "Input/InputDeviceHandler.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Platform/Windows/Win32Window.h"
#include "Input/HID.h"

Pu::InputDeviceHandler::InputDeviceHandler(void)
	: AnyCursor(nullptr, L"Any", RID_DEVICE_INFO()),
	AnyKeyboard(nullptr, L"Any", RID_DEVICE_INFO())
{
#ifdef _WIN32
	/* Get the number of raw input devices currently active. */
	uint32 deviceCount;
	if (GetRawInputDeviceList(nullptr, &deviceCount, sizeof(RAWINPUTDEVICELIST)))
	{
		Log::Error("Unable to get number of raw input devices '%ls'!", _CrtGetErrorString().c_str());
		return;
	}

	/* Get the handles to the raw input devices. */
	vector<RAWINPUTDEVICELIST> raw(deviceCount);
	if (GetRawInputDeviceList(raw.data(), &deviceCount, sizeof(RAWINPUTDEVICELIST)) == static_cast<uint32>(-1))
	{
		Log::Error("Unable to get raw input devices '%ls'!", _CrtGetErrorString().c_str());
		return;
	}

	/* Convert the raw Windows data to our objects. */
	for (const RAWINPUTDEVICELIST idl : raw) AddWin32InputDevice(idl.hDevice);
#else
	Log::Error("Cannot create input device handler on this platform!");
#endif
}

#ifdef _WIN32
void Pu::InputDeviceHandler::RegisterInputDevicesWin32(const Win32Window & wnd) const
{
	/* Add this input handler to the input event of the window. */
	wnd.OnInputEvent.Add(const_cast<InputDeviceHandler&>(*this), &InputDeviceHandler::HandleWin32InputEvent);
	wnd.InputDeviceAdded.Add(const_cast<InputDeviceHandler&>(*this), &InputDeviceHandler::HandleWin32InputDeviceAdded);
	wnd.InputDeviceRemoved.Add(const_cast<InputDeviceHandler&>(*this), &InputDeviceHandler::HandleWin32InputDeviceRemoved);
	wnd.OnCharInput += [this](const Win32Window&, wchar_t character) { Keyboard::CharInput.Post(AnyKeyboard, character); };

	vector<RAWINPUTDEVICE> devices;

	/* Only add a cursor listener if a cursor is present. */
	if (cursors.size())
	{
		/* Enable add and remove events, we want legacy mouse events for window moves and close button presses etc. */
		RAWINPUTDEVICE cursorDevices =
		{
			_CrtEnum2Int(HIDUsagePage::GenericDesktop),
			_CrtEnum2Int(HIDUsageGenericDesktop::Mouse),
			RIDEV_DEVNOTIFY,
			wnd.hndl
		};

		devices.emplace_back(cursorDevices);
	}

	if (keyboards.size())
	{
		/* Enable add and remove events, we want legacy keyboard events for WM_CHAR. */
		RAWINPUTDEVICE keyboardDevices =
		{
			_CrtEnum2Int(HIDUsagePage::GenericDesktop),
			_CrtEnum2Int(HIDUsageGenericDesktop::Keyboard),
			RIDEV_DEVNOTIFY,
			wnd.hndl
		};

		devices.emplace_back(keyboardDevices);
	}

	/* Register the listeners to the window. */
	if (!RegisterRawInputDevices(devices.data(), static_cast<uint32>(devices.size()), sizeof(RAWINPUTDEVICE)))
	{
		Log::Error("Unable to register raw input devices to window '%ls', '%ls'", wnd.GetTitle().c_str(), _CrtGetErrorString().c_str());
	}
}
void Pu::InputDeviceHandler::HandleWin32InputEvent(const Win32Window &, const RAWINPUT & input)
{
	switch (input.header.dwType)
	{
	case RIM_TYPEMOUSE:
		for (Cursor &cur : cursors)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.mouse);
				break;
			}
		}
		break;
	case RIM_TYPEKEYBOARD:
		for (Keyboard &cur : keyboards)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.keyboard);
				break;
			}
		}
		break;
	default:
		Log::Warning("Unknown type of input event received!");
		break;
	}
}

void Pu::InputDeviceHandler::HandleWin32InputDeviceRemoved(const Win32Window &, HANDLE hndl)
{
	/* Attempt to remove the input device from the cursor list. */
	for (size_t i = 0; i < cursors.size(); i++)
	{
		if (hndl == cursors[i].Hndl)
		{
			Log::Message("Removed HID cursor '%ls'.", cursors[i].Name.c_str());
			cursors.removeAt(i);
			return;
		}
	}

	/* Attempt to remove the input device from the keyboard list. */
	for (size_t i = 0; i < keyboards.size(); i++)
	{
		if (hndl == keyboards[i].Hndl)
		{
			Log::Message("Removed HID keyboard '%ls'.", keyboards[i].Name.c_str());
			keyboards.removeAt(i);
			return;
		}
	}

	/* Attempt to remove the input device from the other list. */
	hids.removeAll([hndl](const InputDevice &cur) { return hndl == cur.Hndl; });
}

void Pu::InputDeviceHandler::AddWin32InputDevice(HANDLE hndl)
{
	/* Make sure we don't add the same HID multiple times. */
	if (cursors.contains([hndl](const Cursor &cur) { return hndl == cur.Hndl; })) return;
	if (keyboards.contains([hndl](const Keyboard &cur) { return hndl == cur.Hndl; })) return;
	if (hids.contains([hndl](const InputDevice &cur) { return hndl == cur.Hndl; })) return;

	/* Get the length of the device name. */
	uint32 nameLength;
	if (GetRawInputDeviceInfo(hndl, RIDI_DEVICENAME, nullptr, &nameLength) < 0)
	{
		Log::Error("Unable to get device name length for input device '%p', '%ls'!", hndl, _CrtGetErrorString().c_str());
		return;
	}

	/* Get the full name of the device. */
	wstring name(nameLength, L' ');
	if (GetRawInputDeviceInfo(hndl, RIDI_DEVICENAME, name.data(), &nameLength) < 0)
	{
		Log::Error("Unable to get device name for input device '%p', '%ls'!", hndl, _CrtGetErrorString().c_str());
		return;
	}

	/* Get the device information. */
	RID_DEVICE_INFO info;
	info.cbSize = sizeof(RID_DEVICE_INFO);
	uint32 cbSize = static_cast<uint32>(info.cbSize);
	if (GetRawInputDeviceInfo(hndl, RIDI_DEVICEINFO, &info, &cbSize) < 0)
	{
		Log::Error("Unable to get device information for input device '%ls', '%ls'!", name.c_str(), _CrtGetErrorString().c_str());
		return;
	}

	/* Handle cursor specific code. */
	if (info.dwType == RIM_TYPEMOUSE)
	{
		/* Create new cursor. */
		const size_t i = cursors.size();
		cursors.emplace_back(Cursor(hndl, name, info));

		/* Add the any cursor handles to the new cursor, cannot be done directly cause then we needed a move assignment operator. */
		cursors[i].Moved.Add(AnyCursor.Moved, &EventBus<const Cursor, Vector2>::Post);
		cursors[i].Button.Add(AnyCursor.Button, &EventBus<const Cursor, ButtonEventArgs>::Post);
		cursors[i].Scrolled.Add(AnyCursor.Scrolled, &EventBus<const Cursor, int16>::Post);

		Log::Message("Added HID cursor '%ls'.", name.c_str());
	}
	else if (info.dwType == RIM_TYPEKEYBOARD)
	{
		/* Create new keyboard. */
		const size_t i = keyboards.size();
		keyboards.emplace_back(Keyboard(hndl, name, info));

		/* Add the any keyboard handles to the new keyboard, cannot be done directly cause then we needed a move assignment operator. */
		keyboards[i].KeyDown.Add(AnyKeyboard.KeyDown, &EventBus<const Keyboard, uint16>::Post);
		keyboards[i].KeyUp.Add(AnyKeyboard.KeyUp, &EventBus<const Keyboard, uint16>::Post);

		Log::Message("Added HID keyboard '%ls'.", name.c_str());
	}
	else if (info.dwType == RIM_TYPEHID)
	{
		hids.emplace_back(InputDevice(hndl, name, InputDeviceType::Other, info));
	}
}
#endif