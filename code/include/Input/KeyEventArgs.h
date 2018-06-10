#pragma once
#include <sal.h>
#include "Keys.h"
#include "KeyState.h"
#include "KeyMods.h"
#include "Core\EnumUtils.h"
#include "Core\Events\EventArgs.h"

namespace Plutonium
{
	/* Defines the arguments for a key event. */
	struct KeyEventArgs
	{
		/* The key that caused the event (might not handle all keys!). */
		Keys Key;
		/* The new state of the key. */
		KeyState Action;
		/* The platform specific code for the key. */
		int ScanCode;
		/* The modifiers that are active during the key press. */
		KeyMods Mods;

		/* Initializes a new instance of the KeyEventArgs object. */
		KeyEventArgs(_In_ int key, _In_ int scancode, _In_ int action, int mods)
			: Key(_CrtInt2Enum<Keys>(key)), Action(_CrtInt2Enum<KeyState>(action)), ScanCode(scancode), Mods(_CrtInt2Enum<KeyMods>(mods))
		{}
	};
}