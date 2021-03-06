#pragma warning(disable:4996)

#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include <cstring>
#include <ctype.h>

size_t Plutonium::strlen(const char * str)
{
	if (!str) return 0;
	return ::strlen(str);
}

size_t Plutonium::strlen(const char32_t * str)
{
	if (!str) return 0;

	size_t len = 0;
	while (str[len] != U'\0') ++len;
	return len;
}

void Plutonium::cpystr(const char * src, char * destination)
{
	char c = *src;
	for (size_t i = 0; c != '\0'; i++, c = src[i]) destination[i] = src[i];
}

void Plutonium::cpystr(const char32_t * src, char32_t * destination)
{
	char32_t c = *src;
	for (size_t i = 0; c != U'\0'; i++, c = src[i]) destination[i] = src[i];
}

void Plutonium::substr(const char * src, size_t start, size_t length, char * result)
{
	/* Copy the specified path of the string over and add a null terminator. */
	for (size_t i = start, j = 0, end = start + length; i < end; i++, j++)
	{
		result[j] = src[i];
	}

	/* Add a null terminator. */
	result[length] = '\0';
}

void Plutonium::substr(const char32_t * src, size_t start, size_t length, char32_t * result)
{
	/* Copy the specified  path of the string over. */
	for (size_t i = start, j = 0, end = start + length; i < end; i++, j++)
	{
		result[j] = src[i];
	}

	/* Add a null terminator. */
	result[length] = '\0';
}

size_t Plutonium::spltstr(const char * src, const char specifier, char ** result, size_t start)
{
	/* Loop through all characters in the string. */
	size_t length = 0, i = start, s = start;
	for (char c = src[i]; c != '\0'; c = src[++i])
	{
		/* Check if the current character is the specifier. */
		if (c == specifier)
		{
			/* Make sure we don't return empty strings. */
			if (i - s > 0) substr(src, s, i - s, result[length++]);
			s = i + 1;
		}
	}

	/* If there is a bit of string left at the end add it. */
	if (s != i) substr(src, s, i - s, result[length++]);
	return length;
}

size_t Plutonium::spltstr(const char32_t * src, const char32_t specifier, char32_t ** result, size_t start)
{
	/* Loop through all characters in the string. */
	size_t length = 0, i = start, s = start;
	for (char32_t c = src[i]; c != U'\0'; c = src[++i])
	{
		/* Check if the current character is the specifier. */
		if (c == specifier)
		{
			/* Make sure we don't return empty strings. */
			if (i - s > 0) substr(src, s, i - s, result[length++]);
			s = i + 1;
		}
	}

	/* If there is a bit of string left at the end add it. */
	if (s != i) substr(src, s, i - s, result[length++]);
	return length;
}

size_t Plutonium::spltstr(const char * src, const char * specifiers, size_t argc, char ** result, size_t start)
{
	/* Loop through all characters in the string. */
	size_t length = 0, i = start, s = start;
	for (char c = src[i]; c != '\0'; c = src[++i])
	{
		/* Loop through all specifiers and check if the current character is one. */
		for (size_t j = 0; j < argc; j++)
		{
			if (c == specifiers[j])
			{
				/* Make sure we don't return empty strings. */
				if (i - s > 0) substr(src, s, i - s, result[length++]);
				s = i + 1;
				break;
			}
		}
	}

	/* If there is a bit of string left at the end add it. */
	if (s != i) substr(src, s, i - s, result[length++]);
	return length;
}

size_t Plutonium::spltstr(const char32_t * src, const char32_t * specifiers, size_t argc, char32_t ** result, size_t start)
{
	/* Loop through all characters in the string. */
	size_t length = 0, i = start, s = start;
	for (char32_t c = src[i]; c != U'\0'; c = src[++i])
	{
		/* Loop through all specifiers and check if the current character is one. */
		for (size_t j = 0; j < argc; j++)
		{
			if (c == specifiers[j])
			{
				/* Make sure we don't return empty strings. */
				if (i - s > 0) substr(src, s, i - s, result[length++]);
				s = i + 1;
				break;
			}
		}
	}

	/* If there is a bit of string left at the end add it. */
	if (s != i) substr(src, s, i - s, result[length++]);
	return length;
}

void Plutonium::mrgstr(const char * first, const char * second, char * result)
{
	/* Get the length of the first string. */
	const size_t flen = strlen(first);

	/* Copy over the strings and add null terminator. */
	cpystr(first, result);
	cpystr(second, result + flen);
	result[flen + strlen(second)] = '\0';
}

void Plutonium::mrgstr(const char32_t * first, const char32_t *second, char32_t * result)
{
	/* Get the length of the first string. */
	const size_t flen = strlen(first);
	const size_t slen = strlen(second);

	/* Copy over the first and second string. */
	for (size_t i = 0; i < flen; i++) result[i] = first[i];
	for (size_t i = 0, j = flen; i < slen; i++, j++) result[j] = second[i];

	/* Add null terminator. */
	result[flen + slen] = U'\0';
}

void Plutonium::mrgstr(const char * first, const char * second, char * result, char seperator)
{
	/* Get the length of the first string. */
	const size_t flen = strlen(first);

	/* Copy over the strings, seperator and add null terminator. */
	cpystr(first, result);
	result[flen] = seperator;
	cpystr(second, result + flen + 1);
	result[flen + strlen(second) + 1] = '\0';
}

void Plutonium::mrgstr(const char32_t * first, const char32_t * second, char32_t * result, char32_t seperator)
{
	/* Get the length of the first string. */
	const size_t flen = strlen(first);
	const size_t slen = strlen(second);

	/* Copy over the first and second string and seperate them with the specified char. */
	for (size_t i = 0; i < flen; i++) result[i] = first[i];
	result[flen] = seperator;
	for (size_t i = 0, j = flen + 1; i < slen; i++, j++) result[j] = second[i];

	/* Add null terminator. */
	result[flen + slen + 1] = U'\0';
}

void Plutonium::mrgstr(const char ** values, size_t argc, char * result)
{
	/* Loop through all strings. */
	size_t len = 0;
	for (size_t i = 0; i < argc; i++)
	{
		/* Copy over the current string. */
		const char *cur = values[i];
		cpystr(cur, result + len);
		len += strlen(cur);
	}

	/* Add null terminator. */
	result[len] = '\0';
}

void Plutonium::mrgstr(const char32_t ** values, size_t argc, char32_t * result)
{
	size_t len = 0;

	/* Loop through all strings. */
	for (size_t i = 0, k = 0; i < argc; i++)
	{
		const char32_t *cur = values[i];
		const size_t curLen = strlen(cur);

		/* Copy iver the current string. */
		for (size_t j = 0; j < curLen; j++, k++) result[k] = cur[j];
		len += curLen;
	}

	/* Add null terminator. */
	result[len] = U'\0';
}

void Plutonium::mrgstr(const char ** values, size_t argc, char * result, char seperator)
{
	/* Loop through all strings. */
	size_t len = 0;
	for (size_t i = 0; i < argc; i++, len++)
	{
		/* Copy over the current string and add seperator. */
		const char *cur = values[i];
		cpystr(cur, result + len);
		len += strlen(cur);
		cpystr(&seperator, result + len);
	}

	/* Replace last seperator with a null terminator. */
	result[__max(0, len - 1)] = '\0';
}

void Plutonium::mrgstr(const char32_t ** values, size_t argc, char32_t * result, char32_t seperator)
{
	size_t len = 0;

	/* Loop through all strings. */
	for (size_t i = 0, k = 0; i < argc; i++)
	{
		const char32_t *cur = values[i];
		const size_t curLen = strlen(cur);

		/* Copy iver the current string and add seperator. */
		for (size_t j = 0; j < curLen; j++, k++) result[k] = cur[j];
		result[k++] = seperator;
		len += curLen + 1;
	}

	/* Replace last seperator with a null terminator. */
	result[__max(0, len - 1)] = U'\0';
}

char * Plutonium::heapstr(const char * src)
{
	/* Allocate memory for string. */
	const size_t len = strlen(src);
	char *result = malloc_s(char, len + 1);

	/* Copy string to heap. */
	for (size_t i = 0; i < len; i++) result[i] = src[i];

	result[len] = '\0';
	return result;
}

char * Plutonium::heapstr(const char32_t * src)
{
	/* Allocate memory for string. */
	const size_t len = strlen(src);
	char *result = malloc_s(char, len + 1);

	/* Copies string to heap. */
	for (size_t i = 0; i < len; i++)
	{
		char32_t cur = src[i];

		/* On debug mode check if we're not losing character information. */
#if defined (DEBUG)
		LOG_WAR_IF(cur > UCHAR_MAX, "Lost char when converting from UTF-32 to ASCII!");
#endif
		result[i] = static_cast<char>(cur);
	}

	result[len] = '\0';
	return result;
}

char32_t * Plutonium::heapwstr(const char * src)
{
	/* Allocate memory for string. */
	const size_t len = strlen(src);
	char32_t *result = malloc_s(char32_t, len + 1);

	/* Copies string to heap. */
	for (size_t i = 0; i < len; i++) result[i] = static_cast<char32_t>(src[i]);
	result[len] = U'\0';
	return result;
}

char32_t * Plutonium::heapwstr(const char32_t * src)
{
	/* Allocate memory for string. */
	const size_t len = strlen(src);
	char32_t *result = malloc_s(char32_t, len + 1);

	/* Copies string to heap. */
	for (size_t i = 0; i < len; i++) result[i] = src[i];
	result[len] = U'\0';
	return result;
}

size_t Plutonium::replstr(char * src, char delimiter, char replacement)
{
	/* Loop through desired part of the string. */
	size_t i = 0, j = 0, end = strlen(src);
	for (char cur = src[j]; j < end && cur != '\0'; cur = src[++j], ++i)
	{
		/* Check if current char is the deliminter. */
		if (cur == delimiter)
		{
			/* If char needs to be removed push the string forward else simply replace character. */
			if (replacement == '\0') src[i] = (++j < end ? src[j] : '\0');
			else src[i] = replacement;
		}
		else src[i] = src[j];
	}

	/* If the last character was the delimter we need to remove one from the length. */
	if (j > end) --i;

	/* Add null terminator and return new length. */
	src[i] = '\0';
	return i;
}

size_t Plutonium::cntchar(const char * src, char delimiter)
{
	/* Default to zero. */
	size_t result = 0;

	/* Loop through string. */
	char c = *src;
	for (size_t i = 0; c != '\0'; i++, c = src[i])
	{
		/* If cur is delimiter add one to result. */
		if (c == delimiter) result++;
	}

	/* Return result. */
	return result;
}

size_t Plutonium::cntchar(const char * src, char * delimiters, size_t argc)
{
	/* Default to zero. */
	size_t result = 0;

	/* Loop through string. */
	char c = *src;
	for (size_t i = 0; c != '\0'; i++, c = src[i])
	{
		for (size_t j = 0; j < argc; j++)
		{
			/* If cur is one of the delimiters add one to result. */
			if (c == delimiters[j]) ++result;
		}
	}

	return result;
}

size_t Plutonium::cntchar(const char32_t * src, char32_t delimiter)
{
	/* Default to zero. */
	size_t result = 0;

	/* Loop through string. */
	char32_t c = *src;
	for (size_t i = 0; c != U'\0'; i++, c = src[i])
	{
		/* If cur is delimiter add one to result. */
		if (c == delimiter) ++result;
	}

	return result;
}

size_t Plutonium::cntchar(const char32_t * src, char32_t * delimiters, size_t argc)
{
	/* Default to zero. */
	size_t result = 0;

	/* Loop through string. */
	char32_t c = *src;
	for (size_t i = 0; c != U'\0'; i++, c= src[i])
	{
		for (size_t j = 0; j < argc; j++)
		{
			/* If cur is one of the delimiters add one to result. */
			if (c == delimiters[j]) ++result;
		}
	}

	return result;
}

bool Plutonium::cntstr(const char * src, const char * predicate)
{
	return !strstr(src, predicate);
}

bool Plutonium::cntstr(const char32_t * src, const char32_t * predicate)
{
	char32_t c = *predicate;
	char32_t t = *src;

	/* Loop through all characters in the source string and only append the test character if they match. */
	for (size_t i = 0, j = 0; i < t != U'\0' && c != U'\0'; i++, j++, t = src[i], c = predicate[j])
	{
		if (t != c) j = 0;
	}

	/* if the test character has reahed the end then we know the substring is present. */
	return c == U'\0';
}

bool Plutonium::eqlstr(const char * str1, const char * str2)
{
	return !strcmp(str1, str2);
}

bool Plutonium::eqlstr(const char32_t * str1, const char32_t * str2)
{
	for (size_t i = 0;; i++)
	{
		char32_t c1 = str1[i];

		if (c1 != str2[i]) return false;
		if (c1 == U'\0') return true;
	}
}

bool Plutonium::nullorempty(const char * str)
{
	return strlen(str) < 1;
}

bool Plutonium::nullorempty(const char32_t * str)
{
	return strlen(str) < 1;
}

void Plutonium::tolower(const char * src, char * result)
{
	char c = *src;
	size_t i = 0;

	/* Loop through string and convert all characters to lowercase via c-function. */
	for (; c != '\0'; i++, c = src[i])
	{
		result[i] = static_cast<char>(::tolower(c));
	}

	/* Add null terminator. */
	result[i] = '\0';
}

void Plutonium::tolower(const char32_t * src, char32_t * result)
{
	char32_t c = *src;
	size_t i = 0;

	/* Loop through string and convert all characters to lowercase via c-function. */
	for (; c != U'\0'; i++, c = src[i])
	{
		result[i] = ::tolower(c);
	}

	/* Add null terminator. */
	result[i] = U'\0';
}