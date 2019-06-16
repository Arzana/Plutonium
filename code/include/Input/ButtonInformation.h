#pragma once
#include"Core/Math/Constants.h" 
#include "Core/Platform/Windows/Windows.h"
#include "Core/EnumUtils.h"
#include "HID.h"

#ifdef _WIN32
#include <hidsdi.h>
#endif

namespace Pu
{
	/* Defines the information available for a pressed or released button. */
	struct ButtonInformation
	{
	public:
		/* Initializes a new instance of a button information object for a button HID. */
		ButtonInformation(_In_ HIDUsageGenericDesktop id)
			: page(_CrtEnum2Int(HIDUsagePage::Button)), min(_CrtEnum2Int(id))
		{}

		/* Gets the HID usage page that this button belongs to. */
		_Check_return_ inline uint16 GetUsagePage(void) const
		{
#ifdef _WIN32
			return page;
#else 
			return 0;
#endif
		}

		/* Gets the HID usage code of this button. */
		_Check_return_ inline uint16 GetUsageStart(void) const
		{
#ifdef _WIN32
			return min;
#else
			return 0;
#endif
		}

		/* Copy constructor. */
		ButtonInformation(_In_ const ButtonInformation&) = default;
		/* Move constructor. */
		ButtonInformation(_In_ ButtonInformation&&) = default;

		/* Copy assignment. */
		_Check_return_ ButtonInformation& operator =(_In_ const ButtonInformation&) = default;
		/* Move assignment. */
		_Check_return_ ButtonInformation& operator =(_In_ ButtonInformation&&) = default;

	private:
		friend class InputDevice;

#ifdef _WIN32
		USAGE page;
		USAGE min;

		ButtonInformation(const HIDP_BUTTON_CAPS &caps)
			: page(caps.UsagePage), min(caps.Range.UsageMin)
		{}
#endif
	};
}