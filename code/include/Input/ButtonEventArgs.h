#pragma once
#include <sal.h>
#include "CursorButtons.h"
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines the information for a cursor button event. */
	struct ButtonEventArgs
		: public EventArgs
	{
	public:
		/* True if the button is pressed, otherwise; false. */
		const bool Down;
		/* Defines the button which invoked the event. */
		const CursorButtons Button;

		/* Initializes a new instance of a button event argument. */
		ButtonEventArgs(_In_ CursorButtons button, _In_ bool down)
			: Down(down), Button(button)
		{}
	};
}