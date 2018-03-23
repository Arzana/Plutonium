#pragma once
#include <cstddef>

/* Gets the size of a literal string. */
#define _CRT_NAMEOF_LEN(x)	(sizeof(_CRT_NAMEOF_RAW(x)) / sizeof(char) - 1)
/* Converts a literal to a string. */
#define _CRT_NAMEOF_RAW(x)	(#x)
/* Converts any variable, function, etc. to a string. */
#define _CRT_NAMEOF(name)	Plutonium::_CrtNameOf<decltype(name)>(_CRT_NAMEOF_RAW(name), _CRT_NAMEOF_LEN(name))

namespace Plutonium
{
	/* Converts any name with a specified length to a string, removing useles chars. */
	_Check_return_ inline constexpr const char* _CrtNameOf(_In_ const char *name, _In_ const size_t length)
	{
		return length == 0 ? name : (
			name[length - 1] == ' ' || name[length - 1] == '.' ||
			name[length - 1] == '>' || name[length - 1] == ':' ||
			name[length - 1] == '&' || name[length - 1] == '*' ||
			name[length - 1] == '+' || name[length - 1] == '~' ||
			name[length - 1] == '-' || name[length - 1] == '!') ?
			&name[length] : _CrtNameOf(name, length - 1);
	}

	/* For some types decltype is needed so we need a templated version as well. */
	template <typename _Ty>
	_Check_return_ inline constexpr const char* _CrtNameOf(_In_ const char *name, _In_ const size_t length)
	{
		return _CrtNameOf(name, length);
	}
}