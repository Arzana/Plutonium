#pragma once
#include "Core/Events/EventArgs.h"
#include "ButtonInformation.h"
#include "Keys.h"

namespace Pu
{
	/* Defines all the information about a button press. */
	struct ButtonEventArgs
		: public EventArgs
	{
	public:
		/* Defines the information available for the specific key. */
		ButtonInformation &Information;

		union
		{
			/* Defines the button's keycode. */
			uint16 KeyCode;
			/* Defines the button's general use key. */
			Keys Key;
		};

		/* Initializes a new instance of a button event args object. */
		ButtonEventArgs(_In_ ButtonInformation &info, uint16 code)
			: Information(info), KeyCode(code)
		{}
	};
}