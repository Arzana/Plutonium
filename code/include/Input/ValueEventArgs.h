#pragma once
#include "Core/Events/EventArgs.h"
#include "ValueInformation.h"

namespace Pu
{
	/* Defines all the information about a button press. */
	struct ValueEventArgs
		: public EventArgs
	{
	public:
		/* Defines the information available for the specific slider. */
		ValueInformation &Information;
		/* Defines the slider's value. */
		float Value;

		/* Initializes a new instance of a slider event args object. */
		ValueEventArgs(_In_ ValueInformation &info, float value)
			: Information(info), Value(value)
		{}
	};
}