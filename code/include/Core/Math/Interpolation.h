#pragma once
#include "Basics.h"

namespace Plutonium
{
	/* Performs nearest-neighbor interpolation between two specified points with a specified amount. */
	_Check_return_ inline float near(_In_ float a, _In_ float b, _In_ float v)
	{
		return v < 0.5f ? a : b;
	}

	/* Performs linear interpolation between two specified points with a specified amount. */
	_Check_return_ inline float lerp(_In_ float a, _In_ float b, _In_ float v)
	{
		return a + (b - a) * v;
	}

	/* Performs inverse linear interpolation between two specified points with a specified point. */
	_Check_return_ inline float ilerp(_In_ float a, _In_ float b, _In_ float v)
	{
		return (v - a) / (b - a);
	}

	/* Performs cubic hermite spline interpolation with specified bounds and derivatives. */
	_Check_return_ inline float hermite(_In_ float a, _In_ float ad, _In_ float b, _In_ float bd, _In_ float v)
	{
		return a + (2.0f * a - 2.0f * b + bd + ad) * cube(v) + (3.0f * b - 3.0f * a - 2.0f * ad - bd) * sqr(v) + ad * v;
	}

	/* Performs cubic hermite spline interpolation with zero for derivatives. */
	_Check_return_ inline float smoothstep(_In_ float v)
	{
		return sqr(v) * (3.0f - 2.0f * v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds and with zero for derivatives. */
	_Check_return_ inline float smoothstep(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, smoothstep(v));
	}

	/* Performs cubic hermite spline interpolation with one for the first derivative and zero for the second. */
	_Check_return_ inline float fadeout(_In_ float v)
	{
		return sqr(v) * (1.0f - 3.0f * v) + v;
	}

	/* Performs cubic hermite spline interpolation with specified bounds and with one for the first derivative and zero for the second. */
	_Check_return_ inline float fadeout(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, fadeout(v));
	}

	/* Performs cubic hermite spline interpolation with zero for the first derivative and one for the second. */
	_Check_return_ inline float fadein(_In_ float v)
	{
		return sqr(v) * 2.0f - cube(v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds and with zero for the first derivative and one for the second. */
	_Check_return_ inline float fadein(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, fadein(v));
	}
}