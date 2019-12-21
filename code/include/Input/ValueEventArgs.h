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
		/* Defines the unnormalized value of the slider. */
		uint64 RawValue;
		/* Defines the slider's value. */
		float Value;

		/* Initializes a new instance of a slider event args object. */
		ValueEventArgs(_In_ ValueInformation &info, _In_ uint64 raw, _In_ float value)
			: Information(info), RawValue(raw), Value(value)
		{}
	};
}