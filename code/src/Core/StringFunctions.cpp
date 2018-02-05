#pragma warning(disable:4996)

#include "Core\StringFunctions.h"
#include "Core\SafeMemory.h"
#include <cstring>

void Plutonium::substr(const char * src, size_t start, size_t length, char * result)
{
	/* Copy the specified  path of the string over and add a null terminator. */
	strncpy(result, src + start, length);
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
			/* make sure we don't return empty strings. */
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
				/* make sure we don't return empty strings. */
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
	strcpy(result, first);
	strcpy(result + flen, second);
	result[flen + strlen(second)] = '\0';
}

void Plutonium::mrgstr(const char * first, const char * second, char * result, char seperator)
{
	/* Get the length of the first string. */
	const size_t flen = strlen(first);

	/* Copy over the strings, seperator and add null terminator. */
	strcpy(result, first);
	result[flen] = seperator;
	strcpy(result + flen + 1, second);
	result[flen + strlen(second) + 1] = '\0';
}

void Plutonium::mrgstr(char ** values, size_t argc, char * result)
{
	/* Loop through all strings. */
	size_t len = 0;
	for (size_t i = 0; i < argc; i++)
	{
		/* Copy over the current string. */
		const char *cur = values[i];
		strcpy(result + len, cur);
		len += strlen(cur);
	}

	/* Add null terminator. */
	result[len] = '\0';
}

void Plutonium::mrgstr(char ** values, size_t argc, char * result, char seperator)
{
	/* Loop through all strings. */
	size_t len = 0;
	for (size_t i = 0; i < argc; i++, len++)
	{
		/* Copy over the current string and add seperator. */
		const char *cur = values[i];
		strcpy(result + len, cur);
		len += strlen(cur);
		strcpy(result + len, &seperator);
	}

	/* Replace last seperator with a null terminator. */
	result[len - 1] = '\0';
}

char * Plutonium::heapstr(const char * src)
{
	/* Allocate memory for string. */
	const size_t len = strlen(src);
	char *result = malloc_s(char, len + 1);

	/* Copy string to heap. */
	strcpy(result, src);
	result[len] = '\0';
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