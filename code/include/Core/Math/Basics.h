#pragma once
#include <cmath> 
#include <functional>
#include "Constants.h"

namespace Pu
{
#pragma region square
	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline int32 sqr(_In_ int32 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline uint32 sqr(_In_ uint32 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline int64 sqr(_In_ int64 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline uint64 sqr(_In_ uint64 v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline float sqr(_In_ float v)
	{
		return v * v;
	}

	/* Raises the input value to the power of two. */
	_Check_return_ constexpr inline double sqr(_In_ double v)
	{
		return v * v;
	}
#pragma endregion
#pragma region cube
	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline int32 cube(_In_ int32 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline uint32 cube(_In_ uint32 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline int64 cube(_In_ int64 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline uint64 cube(_In_ uint64 v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline float cube(_In_ float v)
	{
		return v * v * v;
	}

	/* Raises the input value to the power of three. */
	_Check_return_ constexpr inline double cube(_In_ double v)
	{
		return v * v * v;
	}
#pragma endregion
#pragma region max
	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline int32 max(_In_ int32 a, _In_ int32 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline uint32 max(_In_ uint32 a, _In_ uint32 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline int64 max(_In_ int64 a, _In_ int64 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline uint64 max(_In_ uint64 a, _In_ uint64 b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline float max(_In_ float a, _In_ float b)
	{
		return __max(a, b);
	}

	/* Gets the highest of the two input values. */
	_Check_return_ constexpr inline double max(_In_ double a, _In_ double b)
	{
		return __max(a, b);
	}
#pragma endregion
#pragma region min
	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline int32 min(_In_ int32 a, _In_ int32 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline uint32 min(_In_ uint32 a, _In_ uint32 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline int64 min(_In_ int64 a, _In_ int64 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline uint64 min(_In_ uint64 a, _In_ uint64 b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline float min(_In_ float a, _In_ float b)
	{
		return __min(a, b);
	}

	/* Gets the lowest of the two input values. */
	_Check_return_ constexpr inline double min(_In_ double a, _In_ double b)
	{
		return __min(a, b);
	}
#pragma endregion
#pragma region clamp
	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline int32 clamp(_In_ int32 v, _In_ int32 a, _In_ int32 b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline int64 clamp(_In_ int64 v, _In_ int64 a, _In_ int64 b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline float clamp(_In_ float v, _In_ float a, _In_ float b)
	{
		return max(a, min(b, v));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline double clamp(_In_ double v, _In_ double a, _In_ double b)
	{
		return max(a, min(b, v));
	}
#pragma endregion
#pragma region round clamp
	/* Restricts the input value to the specified range by modulating the value. */
	_Check_return_ constexpr inline int32 rclamp(_In_ int32 v, _In_ int32 a, _In_ int32 b)
	{
		return v + (v > a ? 0 : b) - (v < b ? 0 : b);
	}

	/* Restricts the input value to the specified range by modulating the value. */
	_Check_return_ constexpr inline int64 rclamp(_In_ int64 v, _In_ int64 a, _In_ int64 b)
	{
		return v + (v > a ? 0 : b) - (v < b ? 0 : b);
	}

	/* Restricts the input value to the specified range by modulating the value. */
	_Check_return_ constexpr inline float rclamp(_In_ float v, _In_ float a, _In_ float b)
	{
		return v + (v > a ? 0 : b) - (v < b ? 0 : b);
	}

	/* Restricts the input value to the specified range by modulating the value. */
	_Check_return_ constexpr inline double rclamp(_In_ double v, _In_ double a, _In_ double b)
	{
		return v + (v > a ? 0 : b) - (v < b ? 0 : b);
	}
#pragma endregion
#pragma region safe clamp
	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline int32 sclamp(_In_ int32 v, _In_ int32 a, _In_ int32 b)
	{
		return clamp(v, min(a, b), max(a, b));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline int64 sclamp(_In_ int64 v, _In_ int64 a, _In_ int64 b)
	{
		return clamp(v, min(a, b), max(a, b));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline float sclamp(_In_ float v, _In_ float a, _In_ float b)
	{
		return clamp(v, min(a, b), max(a, b));
	}

	/* Gets the input value restricted to the specified range. */
	_Check_return_ constexpr inline double sclamp(_In_ double v, _In_ double a, _In_ double b)
	{
		return clamp(v, min(a, b), max(a, b));
	}
#pragma endregion
#pragma region rectify
	/* Gets the input value clamped between zero and itself. */
	_Check_return_ constexpr inline int32 rectify(_In_ int32 v)
	{
		return max(0, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ constexpr inline int64 rectify(_In_ int64 v)
	{
		return max(0LL, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ constexpr inline float rectify(_In_ float v)
	{
		return max(0.0f, v);
	}

	/* Gets the input value clamped between zero and itself. */
	_Check_return_ constexpr inline double rectify(_In_ double v)
	{
		return max(0.0, v);
	}
#pragma endregion
#pragma region saturate
	/* Clamps the specified value between zero and one. */
	_Check_return_ constexpr inline int32 saturate(_In_ int32 v)
	{
		return clamp(v, 0, 1);
	}

	/* Clamps the specified value between zero and one. */
	_Check_return_ constexpr inline int64 saturate(_In_ int64 v)
	{
		return clamp(v, 0LL, 1LL);
	}

	/* Clamps the specified value between zero and one. */
	_Check_return_ constexpr inline float saturate(_In_ float v)
	{
		return clamp(v, 0.0f, 1.0f);
	}

	/* Clamps the specified value between zero and one. */
	_Check_return_ constexpr inline double saturate(_In_ double v)
	{
		return clamp(v, 0.0, 1.0);
	}
#pragma endregion
#pragma region miscellaneous float functions 
	/* Gets the integer part of the input value. */
	_Check_return_ inline int32 ipart(_In_ float v)
	{
		return static_cast<int32>(floorf(v));
	}

	/* Gets the rounded integer of the input value. */
	_Check_return_ inline int32 iround(_In_ float v)
	{
		return static_cast<int32>(roundf(v));
	}

	/* Gets the fractional part of the input value. */
	_Check_return_ inline float fpart(_In_ float v)
	{
		return v - floorf(v);
	}

	/* Gets the reciprocal of the input value. */
	_Check_return_ constexpr inline float recip(_In_ float v)
	{
		return 1.0f / v;
	}

	/* Gets the sign of the input value. */
	_Check_return_ constexpr inline float sign(_In_ float v)
	{
		return v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f);
	}

	/* Modifies the input radians to be clamped from 0 to Tau. */
	_Check_return_ inline float modrads(_In_ float rads)
	{
		return fmodf(rads, TAU);
	}

	/* Calculates the specified nth root of the specified value. */
	_Check_return_ inline float nthrt(_In_ float value, _In_ uint32 root)
	{
		return powf(value, recip(static_cast<float>(root)));
	}

	/* Checks if two floats are equal with a specfied error tolerance. */
	_Check_return_ inline bool nrlyeql(_In_ float a, _In_ float b, _In_opt_ float tolerance = EPSILON)
	{
		return fabsf(a - b) < tolerance;
	}

	/* Checks if float a is less or equal with a specified error tolernace to float b. */
	_Check_return_ inline bool lnrlyeql(_In_ float a, _In_ float b, _In_opt_ float tolerance = EPSILON)
	{
		return a < b || nrlyeql(a, b, tolerance);
	}

	/* Checks if float a is greater or equal with a specified error tolernace to float b. */
	_Check_return_ inline bool gnrlyeql(_In_ float a, _In_ float b, _In_opt_ float tolerance = EPSILON)
	{
		return a > b || nrlyeql(a, b, tolerance);
	}
#pragma endregion
#pragma region byte conversion
	/* Converts kilobytes to bytes. */
	_Check_return_ constexpr inline uint64 kb2b(_In_ int32 kilobytes)
	{
		return static_cast<uint64>(kilobytes * 1000.0f);
	}

	/* Converts megabytes to bytes. */
	_Check_return_ constexpr inline uint64 mb2b(_In_ int32 megabytes)
	{
		return static_cast<uint64>(megabytes * 1000000.0f);
	}

	/* Converts gigabytes to bytes. */
	_Check_return_ constexpr inline uint64 gb2b(_In_ int32 gigabytes)
	{
		return static_cast<uint64>(gigabytes * 1000000000.0f);
	}

	/* Converts bytes to kilobytes. */
	_Check_return_ constexpr inline int32 b2kb(_In_ uint64 bytes)
	{
		constexpr float denom = recip(static_cast<float>(1000));
		return static_cast<int32>(bytes * denom);
	}

	/* Converts bytes to megabytes. */
	_Check_return_ constexpr inline int32 b2mb(_In_ uint64 bytes)
	{
		constexpr float denom = recip(static_cast<float>(1000000));
		return static_cast<int32>(bytes * denom);
	}

	/* Converts bytes to gigabytes. */
	_Check_return_ constexpr inline int32 b2gb(_In_ uint64 bytes)
	{
		constexpr float denom = recip(static_cast<float>(1000000000));
		return static_cast<int32>(bytes * denom);
	}
#pragma endregion
#pragma region random
	/* Returns a random interger between the specified minimum and maximum values. */
	_Check_return_ inline int32 random(_In_ int32 min, _In_ int32 max)
	{
		return min + (rand() % (max + 1 - min));
	}

	/* Returns a random float between the specified minimum and maximum values. */
	_Check_return_ inline float random(_In_ float min, _In_ float max)
	{
		return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
	}

	/* Returns a random float between zero and one. */
	_Check_return_ inline float random(void)
	{
		return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	}
#pragma endregion
}

namespace std
{
	/* Combines two hashes together into one hash. */
	_Check_return_ constexpr inline size_t hash_combine(_In_ size_t first, _In_ size_t second)
	{
		return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
	}

	/* Adds a second hash tot he first hash. */
	template <typename hashable_t>
	_Check_return_ inline size_t hash_combine(_In_ size_t hash, _In_ const hashable_t &other)
	{
		return hash_combine(hash, std::hash<hashable_t>{}(other));
	}
}