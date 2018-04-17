#include "Streams\StringParsing.h"
#include "Core\Diagnostics\Logging.h"
#include "Core\Math\Constants.h"
#include <cmath>

/* Checks whether a specific ASCII char is a digit. */
#define IS_DIGIT(x)		(static_cast<Plutonium::uint32>((x) - '0') < static_cast<Plutonium::uint32>(10))
/* Checks whether a specific ASCII char is not a digit. */
#define IS_NO_DIGIT(x)	(static_cast<Plutonium::uint32>((x) - '0') >= static_cast<Plutonium::uint32>(10))

/*
Syoyo Fujita.
tinyobjloader (2012)
https://github.com/syoyo/tinyobjloader
*/
bool Plutonium::tryParseFloat(const char * string, const char * stringEnd, float * result)
{
	/* Check for invalid input. */
	ASSERT_IF(string >= stringEnd, "Invalid string end specified!");

	/* Define result parts. */
	float mantissa = 0.0f, exponent = 0.0f;

	/* Define current specifiers. */
	char sign = '+';
	char const *cur = string;

	/* Define reading parameters. */
	int32 read = 0;
	bool endNotReached = false;

	/* Start parsing. */

	/* Update sign is needed and check for non digit chars for fast early out. */
	if (*cur == '+' || *cur == '-')
	{
		sign = *cur;
		++cur;
	}
	else if (IS_NO_DIGIT(*cur)) return false;

	/* Read integer part. */
	for (endNotReached = cur != stringEnd; endNotReached && IS_DIGIT(*cur); ++cur, ++read, endNotReached = cur != stringEnd)
	{
		mantissa *= 10.0f;
		mantissa += static_cast<int32>(*cur - 0x30);
	}

	/* If nothing was read, return fail. */
	if (read < 1) return false;

	/* Read decimal part. */
	if (endNotReached && *cur == '.')
	{
		++cur;
		read = 1;
		endNotReached = cur != stringEnd;

		for (; endNotReached && IS_DIGIT(*cur); ++cur, ++read, endNotReached = cur != stringEnd)
		{
			/* Avoid using math pow, improves precision. */
			constexpr float pow_lut[] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f };
			constexpr size_t lut_entries = sizeof(pow_lut) / sizeof(float);

			mantissa += static_cast<int32>(*cur - 0x30) * (read < lut_entries ? pow_lut[read] : powf(10.0f, -read));
		}
	}

	/* Read exponent part. */
	if (endNotReached && (*cur == 'e' || *cur == 'E'))
	{
		char expSign = '+';

		++cur;
		read = 0;
		endNotReached = cur != stringEnd;

		/* Update sign if needed. */
		if (endNotReached && (*cur == '+' || *cur == '-'))
		{
			expSign = *cur;
			++cur;
		}
		else if (IS_NO_DIGIT(*cur)) return false;

		for (; endNotReached && IS_DIGIT(*cur); ++cur, ++read, endNotReached = cur != stringEnd)
		{
			exponent *= 10.0f;
			exponent += static_cast<int32>(*cur - 0x30);
		}

		/* Apply exponent sign and check if anything has been read at all. */
		if (read == 0) return false;
		exponent *= (expSign == '+' ? 1.0f : -1.0f);
	}

	/* Assemble result. */
	*result = (sign == '+' ? 1.0f : -1.0f) * (exponent ? ldexpf(mantissa * powf(5.0f, exponent), exponent) : mantissa);
	return true;
}