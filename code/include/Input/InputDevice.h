#pragma once
#include "Core/String.h"
#include "Core/Platform/Windows/Windows.h"

namespace Pu
{
	/* Defines the types of input devices. */
	enum class InputDeviceType
	{
		/* A cursor or mouse. */
		Cursor,
		/* A generic keyboard. */
		Keyboard,
		/* Any other HID device. */
		Other
	};

	/* Defines a single input device. */
	class InputDevice
	{
	public:
		/* The name of the input device. */
		wstring Name;
		/* The type of input device. */
		InputDeviceType Type;

	protected:
		friend class InputDeviceHandler;

#ifdef _WIN32
		/* Defines the information about the RID on Windows. */
		RID_DEVICE_INFO Info;
		HANDLE Hndl;

		/* Initializes a new instance of an input device on Windows. */
		InputDevice(HANDLE hndl, const wstring &name, InputDeviceType type, const RID_DEVICE_INFO &info);
#endif
	};
}