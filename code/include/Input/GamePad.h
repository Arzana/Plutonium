#pragma once
#include "InputDevice.h"
#include "Graphics/Platform/NativeWindow.h"

namespace Pu
{
	/* Defines a helper object to receive game pad (controller) data. */
	class GamePad
		: public InputDevice
	{
	public:
		/* Occurs when the state of the left thumbstick changes. */
		EventBus<const GamePad, Vector2> LeftThumbMoved;
		/* Occurs when the state of the right thumbstick changes. */
		EventBus<const GamePad, Vector2> RightThumbMoved;
		/* Occurs when the state of the left trigger changes. */
		EventBus<const GamePad, float> LeftTriggerMoved;
		/* Occurs when the state of the right trigger changes. */
		EventBus<const GamePad, float> RightTriggerMoved;

		GamePad(_In_ const GamePad&) = delete;
		/* Move constructor. */
		GamePad(_In_ GamePad &&value);

		_Check_return_ GamePad& operator =(_In_ const GamePad&) = delete;
		/* Move assignment. */
		_Check_return_ GamePad& operator =(_In_ GamePad &&other);

	private:
		friend class InputDeviceHandler;

		static ButtonInformation buttonInfo;
		bool dpadStats[4];

#ifdef _WIN32
		GamePad(HANDLE hndl, const wstring &name, const RID_DEVICE_INFO &info);
#endif

		void HandleSliderInput(const InputDevice&, const ValueEventArgs &args);
	};
}