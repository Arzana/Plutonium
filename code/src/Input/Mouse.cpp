#include "Input/Mouse.h"
#include "Input/MouseButtons.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "Graphics/Platform/Windows/Win32Window.h"

const Pu::NativeWindow* Pu::Mouse::lockedWnd = nullptr;
Pu::ButtonInformation Pu::Mouse::buttonInfo = Pu::ButtonInformation(Pu::HIDUsageGenericDesktop::Mouse);

Pu::Vector2 Pu::Mouse::GetPosition(void)
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

Pu::Vector2 Pu::Mouse::GetPosition(const NativeWindow & wnd)
{
	return GetPosition() - wnd.GetPosition();
}

bool Pu::Mouse::IsCursorVisible(void)
{
#ifdef _WIN32
	/* Flags can either be set to zero or suppressed when the cursor is invisible. */
	CURSORINFO info{ sizeof(CURSORINFO) };
	GetCursorInfo(&info);
	return info.flags == CURSOR_SHOWING;
#else
	Log::Error("Cannot check whether the cursor is visible on this platform!");
	return true;
#endif
}

void Pu::Mouse::HideCursor(void)
{
	if (!IsCursorVisible()) return;

#ifdef _WIN32
	::ShowCursor(false);
#else
	Log::Warning("Cannot hide the OS cursor on this platform!");
#endif
}

void Pu::Mouse::ShowCursor(void)
{
	if (IsCursorVisible()) return;

#ifdef _WIN32
	::ShowCursor(true);
#else
	Log::Warning("Cannot show the OS cursor on this platform!");
#endif
}

void Pu::Mouse::LockCursor(const NativeWindow & window)
{
	/* Make sure we remove the handlers to the old window. */
	if (lockedWnd)
	{
		lockedWnd->OnLocationChanged -= &Mouse::ClipMouse;
		lockedWnd->OnSizeChanged -= &Mouse::ClipMouse;
	}

	/* Replace the locked window and clip the mouse. */
	lockedWnd = &window;
	ClipMouse(window, ValueChangedEventArgs<Vector2>(Vector2(), Vector2()));

	/* Add the new handlers to the window. */
	window.OnLocationChanged += &Mouse::ClipMouse;
	window.OnSizeChanged += &Mouse::ClipMouse;
}

void Pu::Mouse::FreeCursor(void)
{
	/* Release the handlers to the window. */
	if (lockedWnd)
	{
		lockedWnd->OnLocationChanged -= &Mouse::ClipMouse;
		lockedWnd->OnSizeChanged -= &Mouse::ClipMouse;
		lockedWnd = nullptr;

#ifdef _WIN32
		/* Disable the clip for the cursor. */
		ClipCursor(nullptr);
#else
		Log::Warning("Unable to unlock OS cursor on this platform!");
#endif
	}
}

void Pu::Mouse::HideAndLockCursor(const NativeWindow & window)
{
	HideCursor();
	LockCursor(window);
}

void Pu::Mouse::ShowAndFreeCursor(void)
{
	ShowCursor();
	FreeCursor();
}

void Pu::Mouse::ClipMouse(const NativeWindow & window, ValueChangedEventArgs<Vector2>)
{
	/* Make sure this event comes from the correct window. */
	if (lockedWnd != &window) return;

#ifdef _WIN32
	const Win32Window &wnd = dynamic_cast<const Win32Window&>(window);

	/* Gets the bottom right corner of the window. */
	const Extent2D extent = wnd.GetSize();
	const int32 titleBarHeight = Win32Window::GetDefaultTitleBarHeight();

	/*
	We need to make sure that the title bar is still accessible when the mouse is clipped to ensure that we can still move the window.
	Currently moving the screen and then locking the cursor doesn't lock the cursor properly but this is a small bug and the current result is better.
	*/
	POINT points[] = { { 0, -titleBarHeight }, { static_cast<LONG>(extent.Width), static_cast<LONG>(extent.Height - titleBarHeight) } };

	/* Convert from window space to screen space. */
	MapWindowPoints(wnd.hndl, nullptr, points, 2);

	/* Construct the clipping rectangle and lock the cursor to it. */
	const RECT clip{ points[0].x, points[0].y, points[1].x, points[1].y };
	ClipCursor(&clip);
#else
	Log::Warning("Unable to lock the OS cursor on this platform!");
#endif
}

#ifdef _WIN32
Pu::Mouse::Mouse(HANDLE hndl, const wstring & name, const RID_DEVICE_INFO & info)
	: InputDevice(hndl, name, InputDeviceType::Cursor, info),
	ID(Info.mouse.dwId), ButtonCount(Info.mouse.dwNumberOfButtons), SampleRate(Info.mouse.dwSampleRate),
	Moved("CursorMoved", true), Scrolled("CursorScrolled")
{}

void Pu::Mouse::HandleWin32Event(const RAWMOUSE & info)
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
		if (info.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Left)));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_1_UP) KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Left)));

		/* Check for right button. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Right)));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_2_UP) KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Right)));

		/* Check for middle button. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Middle)));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_3_UP) KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Middle)));

		/* Check for X button 1. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Extra1)));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_4_UP) KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Extra1)));

		/* Check for X button 2. */
		if (info.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) KeyDown.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Extra2)));
		else if (info.usButtonFlags & RI_MOUSE_BUTTON_5_UP) KeyUp.Post(*this, ButtonEventArgs(buttonInfo, _CrtEnum2Int(MouseButtons::Extra2)));

		/* Check for scroll wheel, delta is stored as a signed value so cast it. */
		if (info.usButtonFlags & RI_MOUSE_HWHEEL) Scrolled.Post(*this, *reinterpret_cast<const int16*>(&info.usButtonData));
	}
}
#endif