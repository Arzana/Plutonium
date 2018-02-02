#pragma once
#include <sal.h>
#include "Keys.h"
#include "KeyState.h"
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
		int Mods;

		/* Initializes a new instance of the KeyEventArgs object. */
		KeyEventArgs(_In_ int key, _In_ int scancode, _In_ int action, int mods)
			: Key(static_cast<Keys>(key)), Action(static_cast<KeyState>(action)), ScanCode(scancode), Mods(mods)
		{}
	};
}