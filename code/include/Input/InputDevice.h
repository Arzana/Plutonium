#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "Core/Events/EventBus.h"
#include "ButtonEventArgs.h"
#include "ValueEventArgs.h"
#include "Core/String.h"

namespace Pu
{
	/* Defines the types of input devices. */
	enum class InputDeviceType
	{
		/* A cursor or mouse. */
		Cursor,
		/* A generic keyboard. */
		Keyboard,
		/* A generic game pad. */
		GamePad,
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

		/* Occurs when a button is released. */
		EventBus<const InputDevice, const ButtonEventArgs&> KeyUp;
		/* Occurs when a button is pressed. */
		EventBus<const InputDevice, const ButtonEventArgs&> KeyDown;
		/* Occurs when a varaible slider's value changes. */
		EventBus<const InputDevice, const ValueEventArgs&> ValueChanged;

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

		/* Gets the amount of button on this HID. */
		_Check_return_ inline size_t GetButtonCount(void) const
		{
			return btnCnt;
		}

	protected:
		friend class InputDeviceHandler;

#ifdef _WIN32
		/* Defines the information about the RID on Windows. */
		RID_DEVICE_INFO Info;
		/* Defines the handle used to indenfity the input device on Windows. */
		HANDLE Hndl;

		/* Initializes a new instance of an input device on Windows. */
		InputDevice(HANDLE hndl, const wstring &deviceInstancePath, InputDeviceType type, const RID_DEVICE_INFO &info);
#endif

		/* Gets the variable value for the specified usage value. */
		_Check_return_ float GetUsageValue(_In_ uint16 usageID) const;

	private:
		size_t btnCnt;

		vector<ButtonInformation> btnCaps;
		vector<ValueInformation> valueCaps;
		vector<vector<bool>> btnStates;
		vector<float> valueStates;

#ifdef _WIN32
		PHIDP_PREPARSED_DATA data;
		vector<USAGE> tmpUsageList;
		
		void SetDefaultName(const wstring &deviceInstancePath);
		void HandleWin32Event(const RAWHID &info);
#endif

		void GetCapacities();
		void Destroy();
	};
}