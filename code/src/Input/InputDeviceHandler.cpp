#include "Input/InputDeviceHandler.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Platform/Windows/Win32Window.h"
#include "Input/HID.h"

Pu::InputDeviceHandler::InputDeviceHandler(void)
	: AnyCursor(nullptr, L"Any", RID_DEVICE_INFO())
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
	hids.reserve(deviceCount);
	for (const RAWINPUTDEVICELIST idl : raw)
	{
		/* Get the length of the device name. */
		uint32 nameLength;
		if (GetRawInputDeviceInfo(idl.hDevice, RIDI_DEVICENAME, nullptr, &nameLength) < 0)
		{
			Log::Error("Unable to get device name length for input device '%p', '%ls'!", idl.hDevice, _CrtGetErrorString().c_str());
			continue;
		}

		/* Get the full name of the device. */
		wstring name(nameLength, L' ');
		if (GetRawInputDeviceInfo(idl.hDevice, RIDI_DEVICENAME, name.data(), &nameLength) < 0)
		{
			Log::Error("Unable to get device name for input device '%p', '%ls'!", idl.hDevice, _CrtGetErrorString().c_str());
			continue;
		}

		/* Get the device information. */
		RID_DEVICE_INFO info;
		info.cbSize = sizeof(RID_DEVICE_INFO);
		uint32 cbSize = static_cast<uint32>(info.cbSize);
		if (GetRawInputDeviceInfo(idl.hDevice, RIDI_DEVICEINFO, &info, &cbSize) < 0)
		{
			Log::Error("Unable to get device information for input device '%ls', '%ls'!", name.c_str(), _CrtGetErrorString().c_str());
			continue;
		}

		/* Add the input device to the correct list. */
		switch (idl.dwType)
		{
		case RIM_TYPEMOUSE:
			cursors.emplace_back(Cursor(idl.hDevice, std::move(name), std::move(info)));
			break;
		case RIM_TYPEKEYBOARD:
			//type = InputDeviceType::Keyboard;
			break;
		default:
			hids.emplace_back(InputDevice(idl.hDevice, std::move(name), InputDeviceType::Other, std::move(info)));
			break;
		}
	}
#else
	Log::Error("Cannot create input device handler on this platform!");
#endif

	/* Hook the any cursor into the found cursors. */
	for (const Cursor &cur : cursors)
	{
		cur.Moved.Add(AnyCursor.Moved, &EventBus<const Cursor, Vector2>::Post);
		cur.Button.Add(AnyCursor.Button, &EventBus<const Cursor, ButtonEventArgs>::Post);
		cur.Scrolled.Add(AnyCursor.Scrolled, &EventBus<const Cursor, int16>::Post);
	}
}

#ifdef _WIN32
void Pu::InputDeviceHandler::RegisterInputDevicesWin32(const Win32Window & wnd) const
{
	/* Add this input handler to the input event of the window. */
	wnd.OnInputEvent.Add(const_cast<InputDeviceHandler&>(*this), &InputDeviceHandler::HandleWin32InputEvent);

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
	default:
		Log::Warning("Unknown type of input event received!");
		break;
	}
}
#endif