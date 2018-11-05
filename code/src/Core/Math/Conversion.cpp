#include "Core/Math/Conversion.h"
#include "Core/Diagnostics/Logging.h"

/* Checks whether a specific ASCII char is a digit. */
inline static constexpr bool IsDigit(char c)
{
	return static_cast<Pu::uint32>(c - '0') < 10u;
}

bool Pu::tryParse(const char * begin, const char * end, float * result)
{
	/* On debug check for invalid inputs. */
#ifdef _DEBUG
	if (begin >= end)
	{
		Log::Error("Invalid string end specified!");
		return false;
	}
#endif

	/* Define result parts. */
	float mantissa = 0.0f, exponent = 0.0f;

	/* Define current specifiers. */
	char sign = '+';
	char const *cur = begin;

	/* Define reading parameters. */
	int32 read = 0;
	bool endNotReached = false;

	/* Update sign is needed and check for non digit chars for fast early out. */
	if (*cur == '+' || *cur == '-')
	{
		sign = *cur;
		++cur;
	}
	else if (!IsDigit(*cur)) return false;

	/* Read integer part. */
	for (endNotReached = cur != end; endNotReached && IsDigit(*cur); ++cur, ++read, endNotReached = cur != end)
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
		endNotReached = cur != end;

		for (; endNotReached && IsDigit(*cur); ++cur, ++read, endNotReached = cur != end)
		{
			/* Avoid using math pow, improves precision. */
			constexpr float pow_lut[] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f };
			constexpr size_t lut_entries = sizeof(pow_lut) / sizeof(float);

			mantissa += static_cast<int32>(*cur - 0x30) * (read < lut_entries ? pow_lut[read] : powf(10.0f, -static_cast<float>(read)));
		}
	}

	/* Read exponent part. */
	if (endNotReached && (*cur == 'e' || *cur == 'E'))
	{
		char expSign = '+';

		++cur;
		read = 0;
		endNotReached = cur != end;

		/* Update sign if needed. */
		if (endNotReached && (*cur == '+' || *cur == '-'))
		{
			expSign = *cur;
			++cur;
		}
		else if (!IsDigit(*cur)) return false;

		for (; endNotReached && IsDigit(*cur); ++cur, ++read, endNotReached = cur != end)
		{
			exponent *= 10.0f;
			exponent += static_cast<int32>(*cur - 0x30);
		}

		/* Apply exponent sign and check if anything has been read at all. */
		if (read == 0) return false;
		exponent *= (expSign == '+' ? 1.0f : -1.0f);
	}

	/* Assemble result. */
	*result = (sign == '+' ? 1.0f : -1.0f) * (exponent ? ldexpf(mantissa * powf(5.0f, exponent), static_cast<int>(exponent)) : mantissa);
	return true;
}