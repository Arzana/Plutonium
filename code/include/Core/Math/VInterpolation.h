#pragma once
#include "Interpolation.h"
#include "Vector2.h"
#include "Vector4.h"

namespace Plutonium
{
	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector2 near(_In_ Vector2 a, _In_ Vector2 b, _In_ float v)
	{
		return Vector2(near(a.X, b.X, v), near(a.Y, b.Y, v));
	}

	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector3 near(_In_ Vector3 a, _In_ Vector3 b, _In_ float v)
	{
		return Vector3(near(a.X, b.X, v), near(a.Y, b.Y, v), near(a.Z, b.Z, v));
	}

	/* Performs nearest-neighbor interpolation between two specified points (a, b) with a specified amount (v). */
	_Check_return_ inline Vector4 near(_In_ Vector4 a, _In_ Vector4 b, _In_ float v)
	{
		return Vector4(near(a.X, b.X, v), near(a.Y, b.Y, v), near(a.Z, b.Z, v), near(a.W, b.W, v));
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
	_Check_return_ inline float ilerp(_In_ Vector2 a, _In_ Vector2 b, _In_ Vector2 v)
	{
		Vector2 ab = b - a;
		return dot(v - a, ab) / dot(ab, ab);
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector3 a, _In_ Vector3 b, _In_ Vector3 v)
	{
		Vector3 ab = b - a;
		return dot(v - a, ab) / dot(ab, ab);
	}

	/* Performs inverse linear interpolation between two specified points (a, b) with a specified point (v). */
	_Check_return_ inline float ilerp(_In_ Vector4 a, _In_ Vector4 b, _In_ Vector4 v)
	{
		Vector4 ab = b - a;
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