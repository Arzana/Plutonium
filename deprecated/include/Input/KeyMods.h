#pragma once

namespace Plutonium
{
	/* Defines modifier values for key action events. */
	enum class KeyMods : int
	{
		/* Set if any shift keys are down. */
		Shift = 1,
		/* Set if any control keys are down. */
		Control = 2,
		/* Set if any alt keys are down. */
		Alt = 4,
		/* Set if any super keys are down. */
		Super = 8
	};
}