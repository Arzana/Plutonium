#pragma once
#include <sal.h>

namespace Pu
{
	/* Attempts to convert the specified string to a floating point value. */
	_Check_return_ bool tryParse(_In_ const char *begin, const char *end, _Out_ float *result);
}