#pragma once
#include <cmath>
#include "Constants.h"

namespace Plutonium
{
	/* Raises the input value to the power of two. */
	_Check_return_ inline int32 sqr(_In_ int32 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ inline int64 sqr(_In_ int64 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ inline float sqr(_In_ float v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ inline double sqr(_In_ double v)
	{
		return v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ inline int32 cube(_In_ int32 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ inline int64 cube(_In_ int64 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ inline float cube(_In_ float v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ inline double cube(_In_ double v)
	{
		return v * v * v;
	}

	/* Gets the highest of the two input values. */
	_Check_return_ inline int32 max(_In_ int32 a, _In_ int32 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ inline int64 max(_In_ int64 a, _In_ int64 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ inline float max(_In_ float a, _In_ float b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ inline double max(_In_ double a, _In_ double b)
	{
		return __max(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ inline int32 min(_In_ int32 a, _In_ int32 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ inline int64 min(_In_ int64 a, _In_ int64 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ inline float min(_In_ float a, _In_ float b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ inline double min(_In_ double a, _In_ double b)
	{
		return __min(a, b);
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ inline int32 clamp(_In_ int32 v, _In_ int32 a, _In_ int32 b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ inline int64 clamp(_In_ int64 v, _In_ int64 a, _In_ int64 b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ inline float clamp(_In_ float v, _In_ float a, _In_ float b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ inline double clamp(_In_ double v, _In_ double a, _In_ double b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ inline int32 rectify(_In_ int32 v)
	{
		return max(0, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ inline int64 rectify(_In_ int64 v)
	{
		return max(0LL, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ inline float rectify(_In_ float v)
	{
		return max(0.0f, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ inline double rectify(_In_ double v)
	{
		return max(0.0, v);
	}

	/* Gets the integer part of the input value. */
	_Check_return_ inline int32 ipart(_In_ float v)
	{
		return static_cast<int32>(floorf(v));
	}

	/* Gets the fractional part of the input value. */
	_Check_return_ inline float fpart(_In_ float v)
	{
		return v - floorf(v);
	}

	/* Gets the reciprocal of the input value. */
	_Check_return_ inline float recip(_In_ float v)
	{
		return 1.0f / v;
	}

	/* Modifies the input radians to be clamped from 0 to Tau. */
	_Check_return_ inline float modrads(_In_ float rads)
	{
		return fmodf(rads, TAU);
	}

	/* Converts bytes to kilobytes. */
	_Check_return_ inline int32 b2kb(_In_ uint64 bytes)
	{
		return static_cast<int>(bytes / 1024UL);
	}

	/* Converts bytes to megabytes. */
	_Check_return_ inline int32 b2mb(_In_ uint64 bytes)
	{
		return static_cast<int>(bytes / 1048576UL);
	}

	/* Converts bytes to gigabytes. */
	_Check_return_ inline int32 b2gb(_In_ uint64 bytes)
	{
		return static_cast<int>(bytes / 1073741824UL);
	}
}