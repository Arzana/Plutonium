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
	_Check_return_ inline wstring _CrtGetFileExtension(_In_ wstring path)
	{
		const size_t offset = path.find_last_of(L'.');
		return offset != wstring::npos ? path.substr(offset + 1, path.length() - offset - 1) : L"";
	}

	/* Gets the file name from a string. */
	_Check_return_ inline wstring _CrtGetFileName(_In_ wstring path)
	{
		const size_t start = path.find_last_of({ L'/', L'\\' }) + 1;
		return path.substr(start, path.length() - start);
	}

	/* Gets the file name from a string (without extension). */
	_Check_return_ inline wstring _CrtGetFileNameWithoutExtension(_In_ wstring path)
	{
		const size_t start = path.find_last_of({ L'/', L'\\' }) + 1;
		size_t end = path.find_last_of(L'.');
		if (end == wstring::npos) end = path.length();
		return path.substr(start, end - start);
	}

	/* Gets the file directory from a string. */
	_Check_return_ inline wstring _CrtGetFileDirectory(_In_ wstring path)
	{
		const size_t len = path.find_last_of({ L'/', L'\\' });
		return len != wstring::npos ? path.substr(0, len) : L"";
	}
}