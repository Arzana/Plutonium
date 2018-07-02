#pragma once

namespace Plutonium
{
	/* Defines the available types of cursor buttons. */
	enum class CursorButtons
	{
		/* Defines a default button press (handled by GuiItem). */
		Default,
		/* Defines a left button press (handled by Button). */
		Left,
		/* Defines a right button press (handled by Button). */
		Right, 
		/* Defines a double button press (handled by Button). */
		Double
	};
}