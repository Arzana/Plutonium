#pragma once
#include "Core/Events/EventArgs.h"
#include "ButtonInformation.h"

namespace Pu
{
	/* Defines all the information about a button press. */
	struct ButtonEventArgs
		: public EventArgs
	{
	public:
		/* Defines the information available for the specific key. */
		ButtonInformation &Information;
		/* Defines the button's keycode. */
		uint16 KeyCode;

		/* Initializes a new instance of a button event args object. */
		ButtonEventArgs(_In_ ButtonInformation &info, uint16 code)
			: Information(info), KeyCode(code)
		{}
	};
}