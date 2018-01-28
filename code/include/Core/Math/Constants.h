#pragma once
#include <cstdint>

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
}