#pragma once
#include <sal.h>
#include <Core\Math\Basics.h>

namespace Plutonium
{
	/* Defines a two dimentional vector. */
	typedef struct Vector2
	{
		/* [0, 0] */
		const static Vector2 Zero;
		/* [1, 0] */
		const static Vector2 UnitX;
		/* [0, 1] */
		const static Vector2 UnitY;
		/* [1, 1] */
		const static Vector2 One;

		union
		{
			/* Both components of the vector. */
			float f[2];

			struct
			{
				/* The X component of the vector. */
				float X;
				/* The Y component of the vector. */
				float Y;
			};
		};

		/* Initializes a new instance of a two dimentional vector with both components set to zero. */
		Vector2(void)
			: X(0.0f), Y(0.0f)
		{}

		/* Initializes a new instance of a two dimentional vector with both components set to a specified value. */
		Vector2(_In_ float v)
			: X(v), Y(v)
		{}

		/* Initializes a new instance of a two dimentiona vector with both components specified. */
		Vector2(_In_ float x, _In_ float y)
			: X(x), Y(y)
		{}

		/* Negates the vector. */
		_Check_return_ inline Vector2 operator -(void) const
		{
			return Vector2(-X, -Y);
		}

		/* Subtracts the input vector from the vector. */
		_Check_return_ inline Vector2 operator -(_In_ Vector2 v) const
		{
			return Vector2(X - v.X, Y - v.Y);
		}

		/* Adds the input vector to the vector. */
		_Check_return_ inline Vector2 operator +(_In_ Vector2 v) const
		{
			return Vector2(X + v.X, Y + v.Y);
		}

		/* Multiplies the vector by a scalar value. */
		_Check_return_ inline Vector2 operator *(_In_ float v) const
		{
			return Vector2(X * v, Y * v);
		}

		/* Multiples the vector by another vector. */
		_Check_return_ inline Vector2 operator *(_In_ Vector2 v) const
		{
			return Vector2(X * v.X, Y * v.Y);
		}

		/* Divides the vector by a scalar value. */
		_Check_return_ inline Vector2 operator /(_In_ float v) const
		{
			return Vector2(X / v, Y / v);
		}

		/* Divides the vector by another vector. */
		_Check_return_ inline Vector2 operator /(_In_ Vector2 v) const
		{
			return Vector2(X / v.X, Y / v.Y);
		}

		/* Subtracts the input vector from the vector. */
		inline Vector2 operator -=(_In_ Vector2 v)
		{
			X -= v.X;
			Y -= v.Y;
			return *this;
		}

		/* Adds the input vector to the vector. */
		inline Vector2 operator +=(_In_ Vector2 v)
		{
			X += v.X;
			Y += v.Y;
			return *this;
		}

		/* Multiplies the vector by a scalar value. */
		inline Vector2 operator *=(_In_ float v)
		{
			X *= v;
			Y *= v;
			return *this;
		}

		/* Multiplies the vector by another vector. */
		inline Vector2 operator *=(_In_ Vector2 v)
		{
			X *= v.X;
			Y *= v.Y;
			return *this;
		}

		/* Divides the vector by a scalar value. */
		inline Vector2 operator /=(_In_ float v)
		{
			X /= v;
			Y /= v;
			return *this;
		}

		/* Divides the vector by another vector. */
		inline Vector2 operator /=(_In_ Vector2 v)
		{
			X /= v.X;
			Y /= v.Y;
			return *this;
		}

		/* Checks whether the input vector is equal to the vector. */
		_Check_return_ inline bool operator ==(_In_ Vector2 v) const
		{
			return X == v.X && Y == v.Y;
		}

		/* Checks whether the input vector differs from the vector. */
		_Check_return_ inline bool operator !=(_In_ Vector2 v) const
		{
			return X != v.X || Y != v.Y;
		}

		/* Initializes a new instance of a two dimentional vector from a specified angle. */
		_Check_return_ static inline Vector2 FromAngle(_In_ float theta)
		{
			return Vector2(cosf(theta), sinf(theta));
		}

		/* Gets the angle that the vector defines. */
		_Check_return_ inline float Angle(void) const
		{
			return atan2f(Y, X);
		}

		/* Gets the magnetude of the vector squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return X * X + Y * Y;
		}

		/* Gets the magnetude of the vector. */
		_Check_return_ inline float Length(void) const
		{
			return sqrtf(LengthSquared());
		}

		/* Normalizes the current vector. */
		inline void Normalize(void)
		{
			operator/=(Length());
		}
	} Vec2;

	/* Multiplies the vector by a scalar value. */
	_Check_return_ inline Vector2 operator *(_In_ float s, _In_ Vector2 v)
	{
		return v * s;
	}

	/* Gets the absolute value of each vector component. */
	_Check_return_ inline Vector2 abs(_In_ Vector2 v)
	{
		return Vector2(fabsf(v.X), fabsf(v.Y));
	}

	/* Gets the angle between two specified vectors. */
	_Check_return_ inline float angle(_In_ Vector2 v, _In_ Vector2 w)
	{
		return w.Angle() - v.Angle();
	}

	/* Gets the input vector restricted to the specified range. */
	_Check_return_ inline Vector2 clamp(_In_ Vector2 v, _In_ Vector2 a, _In_ Vector2 b)
	{
		return Vector2(clamp(v.X, a.X, b.X), clamp(v.Y, a.Y, b.Y));
	}

	/* Gets the distance between the two specified vectors. */
	_Check_return_ inline float dist(_In_ Vector2 v, _In_ Vector2 w)
	{
		return (w - v).Length();
	}

	/* Calculates the dot product of the two specified vectors. */
	_Check_return_ inline float dot(_In_ Vector2 v, _In_ Vector2 w)
	{
		return v.X * w.X + v.Y * w.Y;
	}

	/*Gets the highest value from each component from the specified vectors. */
	_Check_return_ inline Vector2 max(_In_ Vector2 v, _In_ Vector2 w)
	{
		return Vector2(__max(v.X, w.X), __max(v.Y, w.Y));
	}

	/* Gets the lowest value from each component from the specified vectors. */
	_Check_return_ inline Vector2 min(_In_ Vector2 v, _In_ Vector2 w)
	{
		return Vector2(__min(v.X, w.X), __min(v.Y, w.Y));
	}

	/* Gets a vector with unit length that has the same direction as the input vector. */
	_Check_return_ inline Vector2 normalize(_In_ Vector2 v)
	{
		return v / v.Length();
	}

	/* Calculates the prep dot product of the two specified vectors. */
	_Check_return_ inline float prepdot(_In_ Vector2 v, _In_ Vector2 w)
	{
		return v.X * w.Y - v.Y * w.X;
	}

	/* Calculates the reflection vector of the specified input vector around the specified normal. */
	_Check_return_ inline Vector2 reflect(_In_ Vector2 v, _In_ Vector2 n)
	{
		return v - 2.0f * dot(v, n) * n;
	}

	/* Gets the sign of each component of the input vector. */
	_Check_return_ inline Vector2 sign(_In_ Vector2 v)
	{
		return Vector2(sign(v.X), sign(v.Y));
	}

	/* Get the input vector restricted to the specified range in positive and negative direction. */
	_Check_return_ inline Vector2 mclamp(_In_ Vector2 v, _In_ Vector2 a, _In_ Vector2 b)
	{
		return clamp(abs(v), a, b) * sign(v);
	}
}