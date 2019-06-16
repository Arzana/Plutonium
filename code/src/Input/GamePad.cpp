#include "Input/GamePad.h"
#include "Input/HID.h"
#include "Input/Keys.h"

Pu::ButtonInformation Pu::GamePad::buttonInfo = Pu::ButtonInformation(Pu::HIDUsageGenericDesktop::GamePad);

#ifdef _WIN32
Pu::GamePad::GamePad(HANDLE hndl, const wstring & name, const RID_DEVICE_INFO & info)
	: InputDevice(hndl, name, InputDeviceType::GamePad, info),
	LeftThumbMoved("GamePadLeftThumbStickMoved"), RightThumbMoved("GamePadRightThumbStickMoved"),
	LeftTriggerMoved("GamePadLeftTriggerMoved"), RightTriggerMoved("GamePadRightTriggerMoved")
{
	ValueChanged.Add(*this, &GamePad::HandleSliderInput);
	memset(dpadStats, false, sizeof(dpadStats));
}
#endif

Pu::GamePad::GamePad(GamePad && value)
	: InputDevice(std::move(value)), 
	LeftThumbMoved(std::move(value.LeftThumbMoved)), RightThumbMoved(std::move(value.RightThumbMoved)),
	LeftTriggerMoved(std::move(value.LeftTriggerMoved)), RightTriggerMoved(std::move(value.RightTriggerMoved))
{
	/* This needs to occur in the move ctor and operator to make sure we don't post an event to a garbage handler. */
	ValueChanged.Remove(value, &GamePad::HandleSliderInput);
	ValueChanged.Add(*this, &GamePad::HandleSliderInput);
	memcpy(dpadStats, value.dpadStats, sizeof(dpadStats));
}

Pu::GamePad & Pu::GamePad::operator=(GamePad && other)
{
	if (this != &other)
	{
		InputDevice::operator=(std::move(other));
		LeftThumbMoved = std::move(other.LeftThumbMoved);
		RightThumbMoved = std::move(other.RightThumbMoved);
		LeftTriggerMoved = std::move(other.LeftTriggerMoved);
		RightTriggerMoved = std::move(other.RightTriggerMoved);
		memcpy(dpadStats, other.dpadStats, sizeof(dpadStats));

		ValueChanged.Remove(other, &GamePad::HandleSliderInput);
		ValueChanged.Add(*this, &GamePad::HandleSliderInput);
	}

	return *this;
}

void Pu::GamePad::HandleSliderInput(const InputDevice&, const ValueEventArgs & args)
{
	/*
	These events occur per dimension for the axis inputs (thumbsticks) so we need to get the last know value for the associated dimension.

	The triggers are stored on the Z axis where the right trigger is range [0, 0.5] and the left trigger is range [0.5, 1].
	We normalize these as well so they're both in [0, 1] range.

	We treat the hat switch as 4 individual buttons so just link to the button down and up events.
	The raw data gives us 9 distinct values, these are all 0.09090909 appart, starting from 0.0.
	*/
	switch (_CrtInt2Enum<HIDUsageGenericDesktop>(args.Information.GetUsageStart()))
	{
	case (HIDUsageGenericDesktop::X):
		LeftThumbMoved.Post(*this, Vector2(args.Value, GetUsageValue(_CrtEnum2Int(HIDUsageGenericDesktop::Y))));
		break;
	case (HIDUsageGenericDesktop::Y):
		LeftThumbMoved.Post(*this, Vector2(GetUsageValue(_CrtEnum2Int(HIDUsageGenericDesktop::X)), args.Value));
		break;
	case (HIDUsageGenericDesktop::Rx):
		RightThumbMoved.Post(*this, Vector2(args.Value, GetUsageValue(_CrtEnum2Int(HIDUsageGenericDesktop::Ry))));
		break;
	case (HIDUsageGenericDesktop::Ry):
		LeftThumbMoved.Post(*this, Vector2(GetUsageValue(_CrtEnum2Int(HIDUsageGenericDesktop::Rx)), args.Value));
		break;
	case (HIDUsageGenericDesktop::Z):
		if (args.Value <= 0.5f) RightTriggerMoved.Post(*this, args.Value * 2.0f);
		else LeftTriggerMoved.Post(*this, (args.Value - 0.5f) * 2.0f);
		break;
	case (HIDUsageGenericDesktop::HatSwitch):
		const int32 check = iround(args.Value / 0.09090909f);

		for (uint16 i = 0, j = 0; i < 4; i++, j += 2)
		{
			if (check && check >= j && check <= j + 2)
			{
				dpadStats[i] = true;
				KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(Keys::XBoxDPadUp) + i));
			}
			else if (dpadStats[i])
			{
				dpadStats[i] = false;
				KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(Keys::XBoxDPadUp) + i));
			}
		}
		break;
	}
}