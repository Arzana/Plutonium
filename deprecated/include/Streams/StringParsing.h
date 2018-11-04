#pragma once
#include <sal.h>

namespace Plutonium
{
	/* Attempts to convert the specified string to a floating point value. */
	_Check_return_ bool tryParseFloat(_In_ const char *string, _In_ const char *stringEnd, _Outptr_ float *result);
}