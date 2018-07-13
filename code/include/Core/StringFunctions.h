#pragma once
#include <sal.h>

namespace Plutonium
{
	/* Gets the length of the specified ASCII string. */
	_Check_return_ size_t strlen(_In_ const char *str);
	/* Gets the length of the specified UTF32 string. */
	_Check_return_ size_t strlen(_In_ const char32_t *str);

	/* Gets a substring from the specified source string. */
	void substr(_In_ const char *src, _In_ size_t start, _In_ size_t length, _Out_ char *result);
	/* Gets a substring from the specified source string. */
	void substr(_In_ const char32_t *src, _In_ size_t start, _In_ size_t length, _Out_ char32_t *result);

	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char *src, _In_ const char specifier, _Out_ char **result, _In_ size_t start);
	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char32_t *src, _In_ const char32_t specifier, _Out_ char32_t **result, _In_ size_t start);
	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char *src, _In_ const char *specifiers, _In_ size_t argc, _Out_ char **result, _In_ size_t start);
	/* Splits a specified string into substrings and copies them to the specified buffer. */
	_Check_return_ size_t spltstr(_In_ const char32_t *src, _In_ const char32_t *specifiers, _In_ size_t argc, _Out_ char32_t **result, _In_ size_t start);

	/* Merges two strings together. */
	void mrgstr(_In_ const char *first, _In_ const char *second, _Out_ char *result);
	/* Merges two strings together. */
	void mrgstr(_In_ const char32_t *first, _In_ const char32_t *second, _Out_ char32_t *result);
	/* Merges two string together with a specified seperator character. */
	void mrgstr(_In_ const char *first, _In_ const char *second, _Out_ char *result, _In_ char seperator);
	/* Merges two string together with a specified seperator character. */
	void mrgstr(_In_ const char32_t *first, _In_ const char32_t *second, _Out_ char32_t *result, _In_ char32_t seperator);
	/* Merges a specified amount of strings together. */
	void mrgstr(_In_ const char **values, _In_ size_t argc, _Out_ char *result);
	/* Merges a specified amount of strings together. */
	void mrgstr(_In_ const char32_t **values, _In_ size_t argc, _Out_ char32_t *result);
	/* Merges a specified amount of string together with a specified seperator character. */
	void mrgstr(_In_ const char **values, _In_ size_t argc, _Out_ char *result, char seperator);
	/* Merges a specified amount of string together with a specified seperator character. */
	void mrgstr(_In_ const char32_t **values, _In_ size_t argc, _Out_ char32_t *result, char32_t seperator);

	/* Copies a string to the heap. */
	_Check_return_ char* heapstr(_In_ const char *src);
	/* Copies a string to the heap. */
	_Check_return_ char32_t* heapwstr(_In_ const char *src);
	/* Copies a string to the heap. */
	_Check_return_ char32_t* heapwstr(_In_ const char32_t *src);

	/* Replaces a specified character in the string with another character. Retuns the new length of the string. */
	_Check_return_ size_t replstr(_Inout_ char *src, _In_ char delimiter, _In_ char replacement);

	/* Counts how many times a specified character apears in a string. */
	_Check_return_ size_t cntchar(_In_ const char *src, _In_ char delimiter);
	/* Counts how many times a specified character apears in a string. */
	_Check_return_ size_t cntchar(_In_ const char32_t *src, _In_ char32_t delimiter);

	/* Checks if the two specified strings are equal. */
	_Check_return_ bool eqlstr(_In_ const char *str1, _In_ const char *str2);
	/* Checks if the two specified strings are equal. */
	_Check_return_ bool eqlstr(_In_ const char32_t *str1, _In_ const char32_t *str2);

	/* Check if the specified string is either null or empty. */
	_Check_return_ bool nullorempty(_In_ const char *str);
	/* Check if the specified string is either null or empty. */
	_Check_return_ bool nullorempty(_In_ const char32_t *str);
}