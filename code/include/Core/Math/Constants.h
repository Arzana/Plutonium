#pragma once
#include <cstdint>
#include <climits>
#include <cfloat>
#include <intrin.h>

namespace Pu
{
	/* Defines a signed 8-bit integer value. */
	using int8 = int8_t;
	/* Defines an unsigned 8-bit integer */
	using uint8 = uint8_t;
	/* Defines a signed 16-big integer.  */
	using int16 = int16_t;
	/* Defines an unsigned 16-bit integer. */
	using uint16 = uint16_t;
	/* Defines a signed 32-bit integer. */
	using int32 = int32_t;
	/* Defines an unsigned 32-bit integer. */
	using uint32 = uint32_t;
	/* Defines a signed 64-bit integer. */
	using int64 = int64_t;
	/* Defines an unsigned 64-bit integer. */
	using uint64 = uint64_t;
	/* Defines an unsigned 8-bit integer. */
	using byte = uint8;
	/* Defines an unsigned 8-bit integer. */
	using octet = uint8;
	/* Defines a UTF-32 character. */
	using char32 = char32_t;
	/* Defines a 128-bit integer vector type. */
	using int128 = __m128i;
	/* Defines a 256-bit integer vector type. */
	using int256 = __m256i;
	/* Defines a 128-bit vector type (quad-float). */
	using qfloat = __m128;
	/* Defines a 256-bit vector type (oct-float). */
	using ofloat = __m256;

	/* Euler's constant. */
	constexpr float E = 2.71828182845904523536f;
	/* The circle constant (180 degrees). */
	constexpr float PI = 3.14159265358979323846f;
	/* PI multiplied by two (360 degrees). */
	constexpr float TAU = PI * 2.0f;
	/* PI divided by two (90 degrees). */
	constexpr float PI2 = PI / 2.0f;
	/* PI divided by four (45 degrees) */
	constexpr float PI4 = PI / 4.0f;
	/* PI divided by eight (22.5 degrees). */
	constexpr float PI8 = PI / 8.0f;
	/* Pythagoras's constant. */
	constexpr float SQRT2 = 1.41421356237309504880f;
	/* SQRT2 divided by two. */
	constexpr float SQRT05 = SQRT2 / 2.0f;
	/*Convertes degrees to radians. */
	constexpr float	DEG2RAD = PI / 180.0f;
	/* Converts radians to degrees. */
	constexpr float	RAD2DEG = 180.0f / PI;
	/* Smallest float unit. */
	constexpr float EPSILON = 0.00000000001f;
	
	/* Defines a union that allows access to the individual floats in a SIMD type. */
	template <typename sse_t, typename single_t>
	union SIMD_UNION
	{
		/* The SIMD type. */
		sse_t SIMD;
		/* The singles. */
		single_t V[sizeof(sse_t) / sizeof(single_t)];
	};

	/* Defines an AVX-float union. */
	using AVX_FLOAT_UNION = SIMD_UNION<ofloat, float>;
	/* Defines an AVX-int union. */
	using AVX_INT_UNION = SIMD_UNION<int256, int32>;
	/* Defines an AVX unsigned-int union. */
	using AVX_UINT_UNION = SIMD_UNION<int256, uint32>;

	/* Gets the minimum value of the specified type. */
	template <typename _Ty>
	_Check_return_ inline constexpr _Ty minv(void)
	{
		return static_cast<_Ty>(0);
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
		return -FLT_MAX;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr double minv<double>(void)
	{
		return -DBL_MAX;
	}

	/* Gets the minimum value of the specified type. */
	template <>
	_Check_return_ inline constexpr long double minv<long double>(void)
	{
		return -LDBL_MAX;
	}

	/* Gets the maximum value of the specified type. */
	template <typename _Ty>
	_Check_return_ inline constexpr _Ty maxv(void)
	{
		static_assert(true, "Cannot get the maximum value of the specified type!");
		return _Ty();
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

	/* Gets the maximum value of the specified type. */
	template <>
	_Check_return_ inline constexpr long double maxv<long double>(void)
	{
		return LDBL_MAX;
	}
}