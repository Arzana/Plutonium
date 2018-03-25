#pragma once
#include <cstdint>
#include <climits>
#include <cfloat>

namespace Plutonium
{
	typedef int8_t		int8;
	typedef uint8_t		uint8;
	typedef int16_t		int16;
	typedef uint16_t	uint16;
	typedef int32_t		int32;
	typedef uint32_t	uint32;
	typedef int64_t		int64;
	typedef uint64_t	uint64;
	typedef uint8		byte;
	typedef uint8		octet;

	/* Euler's constant. */
	constexpr float E = 2.71828182845904523536f;
	/* The circle constant. */
	constexpr float PI = 3.14159265358979323846f;
	/* PI multiplied by two. */
	constexpr float TAU = PI * 2;
	/* PI divided by two. */
	constexpr float PI2 = PI / 2;
	/* PI divided by four */
	constexpr float PI4 = PI / 4;
	/* Pythagoras's constant. */
	constexpr float SQRT2 = 1.41421356237309504880f;
	/*Convertes degrees to radians. */
	constexpr float	DEG2RAD = PI / 180.0f;
	/* Converts radians to degrees. */
	constexpr float	RAD2DEG = 180.0f / PI;
	/* Smallest float unit. */
	constexpr float EPSILON = 0.00000000001f;

	/* Gets the minimum value of the specified type. */
	template <typename _Ty>
	_Check_return_ inline constexpr _Ty minv(void)
	{
		return 0;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int8 minv<int8>(void)
	{
		return SCHAR_MIN;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int16 minv<int16>(void)
	{
		return SHRT_MIN;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int32 minv<int32>(void)
	{
		return INT_MIN;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int64 minv<int64>(void)
	{
		return LLONG_MIN;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr float minv<float>(void)
	{
		return FLT_MIN;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr double minv<double>(void)
	{
		return DBL_MIN;
	}

	/* Gets the maximum value of the specified type. */
	template <typename _Ty>
	_Check_return_ inline constexpr _Ty maxv(void)
	{
		static_assert(true, "Cannot get the maximum value of the specified type!");
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int8 maxv<int8>(void)
	{
		return SCHAR_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr uint8 maxv<uint8>(void)
	{
		return UCHAR_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int16 maxv<int16>(void)
	{
		return SHRT_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr uint16 maxv<uint16>(void)
	{
		return USHRT_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int32 maxv<int32>(void)
	{
		return INT_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr uint32 maxv<uint32>(void)
	{
		return UINT_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr int64 maxv<int64>(void)
	{
		return LLONG_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr uint64 maxv<uint64>(void)
	{
		return ULLONG_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr float maxv<float>(void)
	{
		return FLT_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr double maxv<double>(void)
	{
		return DBL_MAX;
	}
}