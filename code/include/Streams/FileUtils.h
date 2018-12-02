#pragma once
#include "Core/String.h"
#include "Core/Math/Basics.h"

namespace Pu
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

	/* Gets the file extension from a string. */
	_Check_return_ inline string _CrtGetFileExtension(_In_ string path)
	{
		const size_t offset = path.find_last_of('.');
		return offset != string::npos ? path.substr(offset + 1, path.length() - offset - 1) : "";
	}

	/* Gets the file name from a string. */
	_Check_return_ inline string _CrtGetFileName(_In_ string path)
	{
		size_t offset = path.find_last_of({ '/', '\\' });
		if (offset == string::npos) offset = static_cast<size_t>(-1);

		return path.substr(offset + 1, path.length() - offset);
	}

	/* Gets the file name from a string (without extension). */
	_Check_return_ inline string _CrtGetFileNameWithoutExtension(_In_ string path)
	{
		size_t offset = path.find_last_of({ '/', '\\' });
		if (offset == string::npos) offset = 0;

		size_t extLen = path.length() - path.find_last_of('.');
		if (extLen == string::npos) extLen = 0;

		return path.substr(offset, path.length() - offset - extLen);
	}

	/* Gets the file directory from a string. */
	_Check_return_ inline string _CrtGetFileDirectory(_In_ string path)
	{
		const size_t len = path.find_last_of({ '/', '\\' });
		return len != string::npos ? path.substr(0, len) : "";
	}
}