#pragma once
#include <sal.h>
#include <Core\Math\Constants.h>

namespace Plutonium
{
	/* Gets the magic number for a specified string on compile time. */
	_Check_return_ constexpr inline int32 _CrtGetMagicNum(_In_ const char *str, _In_opt_ int32 start = 0)
	{
		/* 
		Loop through string using recursion to allow it to be a constexpr function in C++11. 
		For each character add it's value multiplied by it's offset mutiplied by eight to the result.
		*/
		return str[start] != '\0' ? ((str[start] << (start << 3)) + _CrtGetMagicNum(str, start + 1)) : 0;
	}

	/* Checks if a directory exists. */
	_Check_return_ bool _CrtDirectoryExists(_In_ const char *directory);
	/* Creates a directory if it doesn't excist yet. */
	void _CrtCreateDirectory(_In_ const char *directory);
}