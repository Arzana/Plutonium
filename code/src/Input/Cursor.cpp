#include "Input/Cursor.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"

Pu::Vector2 Pu::Cursor::GetPosition(void)
{
#ifdef _WIN32
	POINT location;
	if (!GetCursorPos(&location))
	{
		Log::Error("Unable to get cursor position '%ls'!", _CrtGetErrorString().c_str());
		return Vector2();
	}

	return Vector2(static_cast<float>(location.x), static_cast<float>(location.y));
#else
	Log::Error("Cannot get cursor position on this platform!");
	return Vector2();
#endif
}

#ifdef _WIN32
Pu::Cursor::Cursor(HANDLE hndl, const wstring & name, const RID_DEVICE_INFO & info)
	: InputDevice(hndl, name, InputDeviceType::Cursor, info),
	ID(Info.mouse.dwId), ButtonCount(Info.mouse.dwNumberOfButtons), SampleRate(Info.mouse.dwSampleRate),
	Moved("CursorMoved"), Button("CursorButtonEvent"), Scrolled("CursorScrolled")
{}

void Pu::Cursor::HandleWin32Event(const RAWMOUSE & info)
{
	/* Handle movement. */
	if (info.lLastX || info.lLastY)
	{
		const Vector2 movement(static_cast<float>(info.lLastX), static_cast<float>(info.lLastY));

		/* Position can either be updated as absolute or relative so handle them correctly. */
		if (info.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			Moved.Post(*this, movement - oldPos);
			oldPos = movement;
		}
		else Moved.Post(*this, movement);
	}

	/* Handle button presses. */
	if (info.usButtonFlags)
	{
		/* Check for left button. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) Button.Post(*this, ButtonEventArgs(CursorButtons::Left, true));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_1_UP) Button.Post(*this, ButtonEventArgs(CursorButtons::Left, false));

		/* Check for right button. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) Button.Post(*this, ButtonEventArgs(CursorButtons::Right, true));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_2_UP) Button.Post(*this, ButtonEventArgs(CursorButtons::Right, false));

		/* Check for middle button. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) Button.Post(*this, ButtonEventArgs(CursorButtons::Middle, true));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_3_UP) Button.Post(*this, ButtonEventArgs(CursorButtons::Middle, false));

		/* Check for X button 1. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) Button.Post(*this, ButtonEventArgs(CursorButtons::Extra1, true));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_4_UP) Button.Post(*this, ButtonEventArgs(CursorButtons::Extra1, false));

		/* Check for X button 2. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) Button.Post(*this, ButtonEventArgs(CursorButtons::Extra2, true));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_5_UP) Button.Post(*this, ButtonEventArgs(CursorButtons::Extra2, false));

		/* Check for scroll wheel, delta is stored as a signed value so cast it. */
		if (info.usButtonFlags & RI_MOUSE_HWHEEL) Scrolled.Post(*this, *reinterpret_cast<const int16*>(&info.usButtonData));
	}
}
#endif