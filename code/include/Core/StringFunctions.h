#pragma once
#include <sal.h>

namespace Plutonium
{
	/* Gets a substring from the specified source string. */
	void substr(_In_ const char *src, _In_ size_t start, _In_ size_t length, _Out_ char *result);
	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char *src, _In_ const char specifier, _Out_ char **result, _In_ size_t start);
	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char *src, _In_ const char *specifiers, _In_ size_t argc, _Out_ char **result, _In_ size_t start);
	/* Merges two strings together. */
	void mrgstr(_In_ const char *first, _In_ const char *second, _Out_ char *result);
	/* Merges two string together with a specified seperator character. */
	void mrgstr(_In_ const char *first, _In_ const char *second, _Out_ char *result, _In_ char seperator);
	/* Merges a specified amount of strings together. */
	void mrgstr(_In_ char **values, _In_ size_t argc, _Out_ char *result);
	/* Merges a specified amount of string together with a specified seperator character. */
	void mrgstr(_In_ char **values, _In_ size_t argc, _Out_ char *result, char seperator);
	/* Copies a string to the heap. */
	_Check_return_ char* heapstr(_In_ const char *src);
	/* Replaces a specified character in the string with another character. Retuns the new length of the string. */
	_Check_return_ size_t replstr(_Inout_ char *src, _In_ char delimiter, _In_ char replacement);
	/* Counts how many times a specified character apears in a string. */
	_Check_return_ size_t cntchar(_In_ const char *src, _In_ char delimiter);
}