#include "Input/InputDeviceHandler.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Platform/Windows/Win32Window.h"
#include "Input/HID.h"

Pu::InputDeviceHandler::InputDeviceHandler(void)
	: AnyKeyDown("InputDeviceHandlerAnyKeyDown", true),
	AnyKeyUp("InputDeviceHandlerAnyKeyUp", true),
	AnyValueChanged("InputDeviceHandlerAnyValueChanged", true),
	AnyMouseScrolled("InputDeviceHandlerAnyMouseScrolled", true),
	AnyMouseMoved("InputDeviceHandlerAnyMouseMoved", true)
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

	vector<RAWINPUTDEVICE> devices;

	/* Enable add and remove events, we want legacy mouse events for window moves and close button presses etc. */
	RAWINPUTDEVICE cursorDevices =
	{
		_CrtEnum2Int(HIDUsagePage::GenericDesktop),
		_CrtEnum2Int(HIDUsageGenericDesktop::Mouse),
		RIDEV_DEVNOTIFY,
		wnd.hndl
	};

	/* Enable add and remove events, we want legacy keyboard events for WM_CHAR. */
	RAWINPUTDEVICE keyboardDevices =
	{
		_CrtEnum2Int(HIDUsagePage::GenericDesktop),
		_CrtEnum2Int(HIDUsageGenericDesktop::Keyboard),
		RIDEV_DEVNOTIFY,
		wnd.hndl
	};

	/* Enable add and remove events. */
	RAWINPUTDEVICE gamepadDevices =
	{
		_CrtEnum2Int(HIDUsagePage::GenericDesktop),
		_CrtEnum2Int(HIDUsageGenericDesktop::GamePad),
		RIDEV_DEVNOTIFY,
		wnd.hndl
	};

	/* Enable precision touchpad add and remove events. */
	RAWINPUTDEVICE touchpadDevices =
	{
		_CrtEnum2Int(HIDUsagePage::Digitizer),
		_CrtEnum2Int(HIDUsageDigitizer::TouchPad),
		RIDEV_DEVNOTIFY,
		wnd.hndl
	};

	devices.emplace_back(cursorDevices);
	devices.emplace_back(keyboardDevices);
	devices.emplace_back(gamepadDevices);
	devices.emplace_back(touchpadDevices);

	/* Register the listeners to the window. */
	if (!RegisterRawInputDevices(devices.data(), static_cast<uint32>(devices.size()), sizeof(RAWINPUTDEVICE)))
	{
		Log::Error("Unable to register raw input devices to window '%ls', '%ls'", wnd.GetTitle().c_str(), _CrtGetErrorString().c_str());
	}
}

void Pu::InputDeviceHandler::HandleWin32InputEvent(const Win32Window &, const RAWINPUT & input)
{
	const char *type;

	switch (input.header.dwType)
	{
	case RIM_TYPEMOUSE:
		for (Mouse &cur : mouses)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.mouse);
				return;
			}
		}

		type = "mouse";
		break;
	case RIM_TYPEKEYBOARD:
		for (Keyboard &cur : keyboards)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.keyboard);
				return;
			}
		}

		type = "keyboard";
		break;
	case RIM_TYPEHID:
		for (GamePad &cur : gamepads)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.hid);
				return;
			}
		}

		for (InputDevice &cur : hids)
		{
			if (cur.Hndl == input.header.hDevice)
			{
				cur.HandleWin32Event(input.data.hid);
				return;
			}
		}

		type = "HID";
		break;
	default:
		Log::Warning("Unknown type of input event received!");
		return;
	}

	/* This should only occur if we could not find the input device in the lists. */
	Log::Warning("Input event received from unknown %s (0x%X)!", type, input.header.hDevice);
}

void Pu::InputDeviceHandler::HandleWin32InputDeviceRemoved(const Win32Window &, HANDLE hndl)
{
	/* Attempt to remove the input device from the cursor list. */
	for (size_t i = 0; i < mouses.size(); i++)
	{
		if (hndl == mouses[i].Hndl)
		{
			mouses.removeAt(i);
			return;
		}
	}

	/* Attempt to remove the input device from the keyboard list. */
	for (size_t i = 0; i < keyboards.size(); i++)
	{
		if (hndl == keyboards[i].Hndl)
		{
			keyboards.removeAt(i);
			return;
		}
	}

	/* Attempt to remove the input device from the gamepad list. */
	for (size_t i = 0; i < gamepads.size(); i++)
	{
		if (hndl == gamepads[i].Hndl)
		{
			gamepads.removeAt(i);
			return;
		}
	}

	/* Attempt to remove the input device from the other list. */
	hids.removeAll([hndl](const InputDevice &cur) { return hndl == cur.Hndl; });
}

void Pu::InputDeviceHandler::AddWin32InputDevice(HANDLE hndl)
{
	/* Make sure we don't add the same HID multiple times. */
	if (mouses.contains([hndl](const Mouse &cur) { return hndl == cur.Hndl; })) return;
	if (keyboards.contains([hndl](const Keyboard &cur) { return hndl == cur.Hndl; })) return;
	if (gamepads.contains([hndl](const GamePad &cur) { return hndl == cur.Hndl; })) return;
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

	InputDevice *device = nullptr;

	/* Handle cursor specific code. */
	if (info.dwType == RIM_TYPEMOUSE)
	{
		/* Create new cursor. */
		mouses.emplace_back(Mouse(hndl, name, info));
		device = &mouses.back();

		/* Add the mouse specific event handlers. */
		mouses.back().Moved.Add(AnyMouseMoved, &decltype(AnyMouseMoved)::Post);
		mouses.back().Scrolled.Add(AnyMouseScrolled, &decltype(AnyMouseScrolled)::Post);
	}
	else if (info.dwType == RIM_TYPEKEYBOARD)
	{
		/* Create new keyboard. */
		const size_t i = keyboards.size();
		keyboards.emplace_back(Keyboard(hndl, name, info));
		device = &keyboards.back();
	}
	else if (info.dwType == RIM_TYPEHID)
	{
		if (info.hid.usUsagePage == _CrtEnum2Int(HIDUsagePage::GenericDesktop) && info.hid.usUsage == _CrtEnum2Int(HIDUsageGenericDesktop::GamePad))
		{
			gamepads.emplace_back(GamePad(hndl, name, info));
			device = &gamepads.back();
		}
		else
		{
			hids.emplace_back(InputDevice(hndl, name, InputDeviceType::Other, info));
			device = &hids.back();
		}
	}

	/* Hook the any key events into the new input device. */
	if (device)
	{
		device->KeyDown.Add(AnyKeyDown, &decltype(AnyKeyDown)::Post);
		device->KeyUp.Add(AnyKeyUp, &decltype(AnyKeyUp)::Post);
		device->ValueChanged.Add(AnyValueChanged, &decltype(AnyValueChanged)::Post);
	}
}
#endif