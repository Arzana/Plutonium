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

	/* Performs cubic interpolation between two specified points (a, c) with a specified amount (v) and a control point (b). */
	_Check_return_ inline constexpr float cubic(_In_ float a, _In_ float b, _In_ float c, _In_ float v)
	{
		const float iv = 1.0f - v;
		return sqr(iv) * a + 2.0f * iv * v * b + sqr(v) * c;
	}

	/* Performs cubic interpolation between two specified points (a, c) with a specified amount (v) and a control point (b). */
	_Check_return_ inline Vector2 cubic(_In_ Vector2 a, _In_ Vector2 b, _In_ Vector2 c, _In_ float v)
	{
		const float iv = 1.0f - v;
		return sqr(iv) * a + 2.0f * iv * v * b + sqr(v) * c;
	}

	/* Performs cubic interpolation between two specified points (a, c) with a specified amount (v) and a control point (b). */
	_Check_return_ inline Vector3 cubic(_In_ Vector3 a, _In_ Vector3 b, _In_ Vector3 c, _In_ float v)
	{
		const float iv = 1.0f - v;
		return sqr(iv) * a + 2.0f * iv * v * b + sqr(v) * c;
	}

	/* Performs quadratic interpolation between two specified points (a, d) with the specified amount (v) and two control points (b, c). */
	_Check_return_ inline constexpr float quadratic(_In_ float a, _In_ float b, _In_ float c, _In_ float d, _In_ float v)
	{
		const float iv = 1.0f - v;
		const float iv2 = sqr(iv);
		const float iv3 = iv2 * iv;
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return iv3 * a + 3.0f * iv2 * v * b + 3.0f * iv * v2 * c + v3 * d;
	}

	/* Performs quadratic interpolation between two specified points (a, d) with the specified amount (v) and two control points (b, c). */
	_Check_return_ inline Vector2 quadratic(_In_ Vector2 a, _In_ Vector2 b, _In_ Vector2 c, _In_ Vector2 d, _In_ float v)
	{
		const float iv = 1.0f - v;
		const float iv2 = sqr(iv);
		const float iv3 = iv2 * iv;
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return iv3 * a + 3.0f * iv2 * v * b + 3.0f * iv * v2 * c + v3 * d;
	}

	/* Performs quadratic interpolation between two specified points (a, d) with the specified amount (v) and two control points (b, c). */
	_Check_return_ inline Vector3 quadratic(_In_ Vector3 a, _In_ Vector3 b, _In_ Vector3 c, _In_ Vector3 d, _In_ float v)
	{
		const float iv = 1.0f - v;
		const float iv2 = sqr(iv);
		const float iv3 = iv2 * iv;
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return iv3 * a + 3.0f * iv2 * v * b + 3.0f * iv * v2 * c + v3 * d;
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

	/* Performs cubic hermite spline interpolation with specified bounds (0, 1) and derivatives (ad, bd). */
	_Check_return_ inline constexpr float hermite(_In_ float ad, _In_ float bd, _In_ float v)
	{
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return (v3 - 2.0f * v2 + v) * ad + (-2.0f * v3 + 3.0f * v2) + (v3 - v2) * bd;
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and derivatives (ad, bd). */
	_Check_return_ inline constexpr float hermite(_In_ float a, _In_ float ad, _In_ float b, _In_ float bd, _In_ float v)
	{
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return (2.0f * v3 - 3.0f * v2 + 1.0f) * a + (v3 - 2.0f * v2 + v) * ad + (-2.0f * v3 + 3.0f * v2) * b + (v3 - v2) * bd;
	}

	/* Performs cubic hermite spline interpolation with zero for derivatives. */
	_Check_return_ inline constexpr float smoothstep(_In_ float v)
	{
		return sqr(v) * (3.0f - 2.0f * v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for derivatives. */
	_Check_return_ inline constexpr float smoothstep(_In_ float a, _In_ float b, _In_ float v)
	{
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return a * (2.0f * v3 - 3.0f * v2 + 1.0f) + b * (-2.0f * v3 + 3.0f * v2);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for derivatives. */
	_Check_return_ inline Vector2 smoothstep(_In_ Vector2 a, _In_ Vector2 b, _In_ float v)
	{
		return Vector2(smoothstep(a.X, b.X, v), smoothstep(a.Y, b.Y, v));
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for derivatives. */
	_Check_return_ inline Vector3 smoothstep(_In_ Vector3 a, _In_ Vector3 b, _In_ float v)
	{
		return Vector3(smoothstep(a.X, b.X, v), smoothstep(a.Y, b.Y, v), smoothstep(a.Z, b.Z, v));
	}

	/* Performs cubic hermite spline interpolation with zero for the first derivative and one for the second. */
	_Check_return_ inline constexpr float fadein(_In_ float v)
	{
		return sqr(v) * 2.0f - cube(v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with zero for the first derivative and one for the second. */
	_Check_return_ inline constexpr float fadein(_In_ float a, _In_ float b, _In_ float v)
	{
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return -v3 + 3.0f * b * v2 - v2 + 2.0f * a * v3 - 3.0f * a * v2 + a;
	}

	/* Performs cubic hermite spline interpolation with one for the first derivative and zero for the second. */
	_Check_return_ inline constexpr float fadeout(_In_ float v)
	{
		return v - cube(v) + sqr(v);
	}

	/* Performs cubic hermite spline interpolation with specified bounds (a, b) and with one for the first derivative and zero for the second. */
	_Check_return_ inline constexpr float fadeout(_In_ float a, _In_ float b, _In_ float v)
	{
		const float v2 = sqr(v);
		const float v3 = v2 * v;
		return -v3 + 3.0f * b * v2 - 2.0f * v2 + v + 2.0f * a * v3 - 3.0f * a * v2 + a;
	}

	/* Gets a dampening factor the specified value of lambda with the specific delta time. */
	_Check_return_ inline float damp(_In_ float lambda, _In_ float dt)
	{
		return 1.0f - expf(-lambda * dt);
	}

	/* Gets a damped interpolation with specified bounds (a, b). */
	_Check_return_ inline float damp(_In_ float a, _In_ float b, _In_ float lambda, _In_ float dt)
	{
		return lerp(a, b, damp(lambda, dt));
	}

	/* Gets a damped interpolation with specified bounds (a, b). */
	_Check_return_ inline Vector2 damp(_In_ Vector2 a, _In_ Vector2 b, _In_ float lambda, _In_ float dt)
	{
		return lerp(a, b, damp(lambda, dt));
	}

	/* Gets a damped interpolation with specified bounds (a, b). */
	_Check_return_ inline Vector3 damp(_In_ Vector3 a, _In_ Vector3 b, _In_ float lambda, _In_ float dt)
	{
		return lerp(a, b, damp(lambda, dt));
	}

	/* Calculates the cartesian coordinate of a point defined by a triangle and two normalized coordinates. */
	_Check_return_ constexpr inline float barycentric(_In_ float p1, _In_ float p2, _In_ float p3, _In_ float a1, _In_ float a2)
	{
		return p1 * a1 + p2 * a2 + p3 * (1.0f - a1 - a2);
	}

	/* Calculates the cartesian coordinate of a point defined by a triangle and two normalized coordinates. */
	_Check_return_ inline Vector2 barycentric(_In_ Vector2 p1, _In_ Vector2 p2, _In_ Vector2 p3, _In_ float a1, _In_ float a2)
	{
		return p1 * a1 + p2 * a2 + p3 * (1.0f - a1 - a2);
	}

	/* Calculates the cartesian coordinate of a point defined by a triangle and two normalized coordinates. */
	_Check_return_ inline Vector3 barycentric(_In_ Vector3 p1, _In_ Vector3 p2, _In_ Vector3 p3, _In_ float a1, _In_ float a2)
	{
		return p1 * a1 + p2 * a2 + p3 * (1.0f - a1 - a2);
	}
}