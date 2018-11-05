#pragma once
#include "Vector4.h"

#if defined(near)
#undef near
#endif

#if defined(far)
#undef far
#endif

namespace Pu
{
	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline constexpr float near(_In_ float a, _In_ float b, _In_ float v)
	{
		return v < 0.5f ? a : b;
	}

	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector2 near(_In_ Vector2 a, _In_ Vector2 b, _In_ float v)
	{
		return v < 0.5f ? a : b;
	}

	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector3 near(_In_ Vector3 a, _In_ Vector3 b, _In_ float v)
	{
		return v < 0.5f ? a : b;
	}

	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector4 near(_In_ Vector4 a, _In_ Vector4 b, _In_ float v)
	{
		return v < 0.5f ? a : b;
	}

	/* Performs linear interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline constexpr float lerp(_In_ float a, _In_ float b, _In_ float v)
	{
		return a + (b - a) * v;
	}

	/* Performs linear interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector2 lerp(_In_ Vector2 a, _In_ Vector2 b, _In_ float v)
	{
		return Vector2(lerp(a.X, b.X, v), lerp(a.Y, b.Y, v));
	}

	/* Performs linear interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector3 lerp(_In_ Vector3 a, _In_ Vector3 b, _In_ float v)
	{
		return Vector3(lerp(a.X, b.X, v), lerp(a.Y, b.Y, v), lerp(a.Z, b.Z, v));
	}

	/* Performs linear interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector4 lerp(_In_ Vector4 a, _In_ Vector4 b, _In_ float v)
	{
		return Vector4(lerp(a.X, b.X, v), lerp(a.Y, b.Y, v), lerp(a.Z, b.Z, v), lerp(a.W, b.W, v));
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline constexpr float ilerp(_In_ float a, _In_ float b, _In_ float v)
	{
		return (v - a) / (b - a);
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector2 a, _In_ Vector2 b, _In_ Vector2 v)
	{
		const Vector2 ab = b - a;
		return dot(v - a, ab) / ab.LengthSquared();
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector3 a, _In_ Vector3 b, _In_ Vector3 v)
	{
		const Vector3 ab = b - a;
		return dot(v - a, ab) / ab.LengthSquared();
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector4 a, _In_ Vector4 b, _In_ Vector4 v)
	{
		const Vector4 ab = b - a;
		return dot(v - a, ab) / ab.LengthSquared();
	}

	/* Performs safe inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline constexpr float ilerp_s(_In_ float a, _In_ float b, _In_ float v)
	{
		return a != b ? ilerp(a, b, v) : 0.0f;
	}

	/* Performs linear interpolation between the current point (a) and the target point (b) over delta time (dt) with a specified speed (s). */
	_Check_return_ inline constexpr float interp(_In_ float a, _In_ float b, _In_ float dt, _In_ float s)
	{
		return lerp(a, b, dt * s);
	}

	/* Performs linear interpolation between the current point (a) and the target point (b) over delta time (dt) with a specified speed (s). */
	_Check_return_ inline Vector2 interp(_In_ Vector2 a, _In_ Vector2 b, _In_ float dt, _In_ float s)
	{
		return lerp(a, b, dt * s);
	}

	/* Performs linear interpolation between the current point (a) and the target point (b) over delta time (dt) with a specified speed (s). */
	_Check_return_ inline Vector3 interp(_In_ Vector3 a, _In_ Vector3 b, _In_ float dt, _In_ float s)
	{
		return lerp(a, b, dt * s);
	}

	/* Performs linear interpolation between the current point (a) and the target point (b) over delta time (dt) with a specified speed (s). */
	_Check_return_ inline Vector4 interp(_In_ Vector4 a, _In_ Vector4 b, _In_ float dt, _In_ float s)
	{
		return lerp(a, b, dt * s);
	}

	/* Maps the input value (range: d-c) to the output specified range (b-a). */
	_Check_return_ inline constexpr float map(_In_ float a, _In_ float b, _In_ float v, _In_ float c, _In_ float d)
	{
		return sclamp(lerp(a, b, ilerp(c, d, v)), a, b);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and derivatives (ad, bd). */
	_Check_return_ inline constexpr float hermite(_In_ float a, _In_ float ad, _In_ float b, _In_ float bd, _In_ float v)
	{
		return a + (2.0f * a - 2.0f * b + bd + ad) * cube(v) + (3.0f * b - 3.0f * a - 2.0f * ad - bd) * sqr(v) + ad * v;
	}

	/* Performs cubic hermite spline interpolation with zero for derivatives. */
	_Check_return_ inline constexpr float smoothstep(_In_ float v)
	{
		return sqr(v) * (3.0f - 2.0f * v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for derivatives. */
	_Check_return_ inline constexpr float smoothstep(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, smoothstep(v));
	}

	/* Performs cubic hermite spline interpolation with one for the first derivative and zero for the second. */
	_Check_return_ inline constexpr float fadeout(_In_ float v)
	{
		return sqr(v) * (1.0f - 3.0f * v) + v;
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with one for the first derivative and zero for the second. */
	_Check_return_ inline constexpr float fadeout(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, fadeout(v));
	}

	/* Performs cubic hermite spline interpolation with zero for the first derivative and one for the second. */
	_Check_return_ inline constexpr float fadein(_In_ float v)
	{
		return sqr(v) * 2.0f - cube(v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for the first derivative and one for the second. */
	_Check_return_ inline constexpr float fadein(_In_ float a, _In_ float b, _In_ float v)
	{
		return lerp(a, b, fadein(v));
	}
}