#pragma once
#include "Interpolation.h"
#include "Vector4.h"
#include "Quaternion.h"

namespace Plutonium
{
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

	/* Performs nearest-neighbor interpolation between two specified rotations (a, b) with a specified amount (v). */
	_Check_return_ inline Quaternion near(_In_ Quaternion a, _In_ Quaternion b, _In_ float v)
	{
		return v < 0.5f ? a : b;
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

	/* Performs linear interpolation between two specified rotations (a, b) with a specified amount (v). */
	_Check_return_ inline Quaternion lerp(_In_ Quaternion a, _In_ Quaternion b, _In_ float v)
	{
		const float iv = 1.0f - v;
		return normalize(dot(a, b) >= 0.0f ? (a * iv + b * v) : (a * iv - b * a));
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector2 a, _In_ Vector2 b, _In_ Vector2 v)
	{
		const Vector2 ab = b - a;
		return dot(v - a, ab) / dot(ab, ab);
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector3 a, _In_ Vector3 b, _In_ Vector3 v)
	{
		const Vector3 ab = b - a;
		return dot(v - a, ab) / dot(ab, ab);
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector4 a, _In_ Vector4 b, _In_ Vector4 v)
	{
		const Vector4 ab = b - a;
		return dot(v - a, ab) / dot(ab, ab);
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
}