#pragma once
#include <sal.h>
#include "Core\Events\EventArgs.h"

namespace Plutonium
{
	/* Defines the arguments for a special button event. */
	struct ButtonEventArgs
		: public EventArgs
	{
		/* 
		The ID for the GLFW button;
		GLFW_MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_2, GLFW_MOUSE_BUTTON_3,
		GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_MIDDLE
		are already handled by the framework and should never occur here.
		*/
		int Button;
		/* True if the button is pressed, otherwise; false. */
		bool Down;
		/* The modifiers that are active during the button press. */
		int Mods;

		/* Initializes a new instance of the ButtonEventArgs object. */
		ButtonEventArgs(_In_ int key, _In_ bool action, _In_ int mods)
			: Button(key), Down(action), Mods(mods)
		{}
	};
}