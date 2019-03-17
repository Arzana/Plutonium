#pragma once
#include "ButtonEventArgs.h"
#include "InputDevice.h"
#include "Core/Math/Vector2.h"
#include "Core/Events/EventBus.h"

namespace Pu
{
	/* Defines a helper object to receive cursor data. */
	class Cursor
		: public InputDevice
	{
	public:
		/* Occurs when the cursor is moved, gives the delta movement. */
		EventBus<const Cursor, Vector2> Moved;
		/* Occurs when the state of a button changed on the cursor. */
		EventBus<const Cursor, ButtonEventArgs> Button;
		/* Occurs when the scroll wheel state changes, gives the delta movement. */
		EventBus<const Cursor, int16> Scrolled;

		/* The unique indentifier of the cursor. */
		const uint64 ID;
		/* The number of unique buttons on the cursor. */
		const size_t ButtonCount;
		/* The number of data pointes per second. */
		const size_t SampleRate;

		Cursor(_In_ const Cursor&) = delete;
		Cursor(_In_ Cursor&&) = default;

		_Check_return_ Cursor& operator =(_In_ const Cursor&) = delete;
		_Check_return_ Cursor& operator =(_In_ Cursor&&) = delete;

		/* Gets the absolute position of the on screen OS cursor. */
		_Check_return_ static Vector2 GetPosition(void);

	private:
		friend class InputDeviceHandler;

		Vector2 oldPos;

#ifdef _WIN32
		Cursor(HANDLE hndl, const wstring &name, const RID_DEVICE_INFO &info);

		void HandleWin32Event(const RAWMOUSE &info);
#endif
	};
}