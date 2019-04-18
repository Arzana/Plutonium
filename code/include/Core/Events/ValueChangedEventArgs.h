#pragma once
#include <sal.h>
#include "EventArgs.h"

namespace Pu
{
	/* Defines a structure to hold the event data for a value change. */
	template <typename value_t>
	struct ValueChangedEventArgs
		: public EventArgs
	{
		/* The old value. */
		const value_t OldValue;
		/* The new value. */
		const value_t NewValue;

		/* Initializes a new instance of the value changed event args structure. */
		ValueChangedEventArgs(_In_ const value_t oldValue, _In_ const value_t newValue)
			: EventArgs(), OldValue(oldValue), NewValue(newValue)
		{}
	};
}