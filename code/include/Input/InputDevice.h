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
		const wstring Name;
		/* The type of input device. */
		const InputDeviceType Type;

		InputDevice(_In_ const InputDevice&) = delete;
		InputDevice(_In_ InputDevice&&) = default;

		_Check_return_ InputDevice& operator =(_In_ const InputDevice&&) = delete;
		_Check_return_ InputDevice& operator =(_In_ InputDevice&&) = delete;

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