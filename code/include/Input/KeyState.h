#pragma once

namespace Plutonium
{
	/* Defines the states a key can be in. */
	enum class KeyState : int
	{
		/* The key is released. */
		Up,
		/* The key is pressed. */
		Down,
		/* The has been pressed for a period of time. */
		Repeat
	};
}