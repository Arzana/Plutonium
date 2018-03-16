#pragma once
#include <sal.h>
#include <Core\Math\Constants.h>

namespace Plutonium
{
	/* Gets the magic number for a specified string on compile time. */
	_Check_return_ constexpr inline int32 _CrtGetMagicNum(_In_ const char *str, _In_ int32 start = 0)
	{
		/* 
		Loop through string using recursion to allow it to be a constexpr function in C++11. 
		For each character add it's value multiplied by it's offset mutiplied by eight to the result.
		*/
		return str[start] != '\0' ? ((str[start] << (start << 3)) + _CrtGetMagicNum(str, start + 1)) : 0;
	}
}