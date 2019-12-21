#pragma once
#include "Core/Math/Constants.h"
#include "Core/Platform/Windows/Windows.h"

#ifdef _WIN32
#include <hidsdi.h>
#endif

namespace Pu
{
	/* Defines the information available for a variable value. */
	struct ValueInformation
	{
	public:
		/* Gets the HID usage page that this value belongs to. */
		_Check_return_ inline HIDUsagePage GetUsagePage(void) const
		{
#ifdef _WIN32
			return _CrtInt2Enum<HIDUsagePage>(page);
#else 
			return HIDUsagePage::Undefined;;
#endif
		}

		/* Gets the HID usage code of this value. */
		_Check_return_ inline uint16 GetUsageStart(void) const
		{
#ifdef _WIN32
			return min;
#else
			return 0;
#endif
		}

		/* Copy constructor. */
		ValueInformation(_In_ const ValueInformation&) = default;
		/* Move constructor. */
		ValueInformation(_In_ ValueInformation&&) = default;

		/* Copy assignment. */
		_Check_return_ ValueInformation& operator =(_In_ const ValueInformation&) = default;
		/* Move assignment. */
		_Check_return_ ValueInformation& operator =(_In_ ValueInformation&&) = default;

	private:
		friend class InputDevice;

#ifdef _WIN32
		USAGE page;
		USAGE min;

		float valueMin;
		float valueMax;

		ValueInformation(const HIDP_VALUE_CAPS &caps)
			: page(caps.UsagePage), min(caps.Range.UsageMin)
		{
			union SignedToUnsigned
			{
				LONG Signed;
				ULONG Unsigned;
			} cnvrt;

			const ULONG mask = (1 << caps.BitSize) - 1;
			
			cnvrt.Signed = caps.PhysicalMin;
			valueMin = static_cast<float>(cnvrt.Unsigned & mask);

			cnvrt.Signed = caps.PhysicalMax;
			valueMax = static_cast<float>(cnvrt.Unsigned & mask);
		}
#endif
	};
}