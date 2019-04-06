#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "Core/Platform/DynamicLibLoader.h"

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

		InputDevice(const InputDevice&) = delete;
		/* Move constructor. */
		InputDevice(InputDevice &&value);
		/* Reallocates the resource allocated by the input device. */
		virtual ~InputDevice()
		{
			Destroy();
		}

		_Check_return_ InputDevice& operator =(_In_ const InputDevice&) = delete;
		/* Move assignment. */
		_Check_return_ InputDevice& operator =(_In_ InputDevice &&other);

	protected:
		friend class InputDeviceHandler;

#ifdef _WIN32
		/* Defines the information about the RID on Windows. */
		RID_DEVICE_INFO Info;
		HANDLE Hndl;

		/* Initializes a new instance of an input device on Windows. */
		InputDevice(HANDLE hndl, const wstring &deviceInstancePath, InputDeviceType type, const RID_DEVICE_INFO &info);

#endif

	private:
#ifdef _WIN32
		using PFN_HidD_GetProductString = BOOLEAN(*)(HANDLE, PVOID, ULONG);

		static DynamicLibLoader hidLibLoader;
		static PFN_HidD_GetProductString HidD_GetProductString;

		void TrySetName(HANDLE hHid);
#endif

		bool nameSet;

		void Init();
		void Destroy();
	};
}