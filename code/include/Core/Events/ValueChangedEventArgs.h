#pragma once
#include <sal.h>
#include "EventArgs.h"

namespace Pu
{
	/* Defines a structure to hold the event data for a value change. */
	template <typename _Ty>
	struct ValueChangedEventArgs
		: public EventArgs
	{
		/* The old value. */
		const _Ty OldValue;
		/* The new value. */
		const _Ty NewValue;

		/* Initializes a new instance of the value changed event args structure. */
		ValueChangedEventArgs(_In_ const _Ty oldValue, _In_ const _Ty newValue)
			: EventArgs(), OldValue(oldValue), NewValue(newValue)
		{}
	};
}