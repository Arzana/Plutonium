#pragma once
#include "Point.h"
#include "Basics.h"
#include "Core/String.h"

namespace Pu
{
	/* Defines a two dimensional vector. */
	struct Vector2
	{
	public:
		union
		{
			/* Specifies both components of the vector. */
			float f[2];

			/* This project is meant to be compiled with the Microsoft compiler which allows this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
			struct
			{
				/* The X component of the vector. */
				float X;
				/* The Y component of the vector. */
				float Y;
			};
#pragma warning(pop)
		};

		/* Initializes a new instance of a two dimensional vector with both components set to zero. */
		Vector2(void)
			: X(0.0f), Y(0.0f)
		{}

		/* Initializes a new instance of a two dimensional vector with both components set to a specified value. */
		Vector2(_In_ float v)
			: X(v), Y(v)
		{}

		/* Initializes a new instance of a two dimensional vector with both components specified. */
		Vector2(_In_ float x, _In_ float y)
			: X(x), Y(y)
		{}

		/* Initializes a new instance of a two dimensional vector from a point. */
		template <typename component_t>
		Vector2(_In_ GenPoint<component_t> p)
			: X(static_cast<float>(p.X)), Y(static_cast<float>(p.Y))
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

		/* Implicitly converts the 2D vector to a string. */
		_Check_return_ inline operator string() const
		{
			string result("[X: ");
			result += string::from(X);
			result += ", Y: ";
			result += string::from(Y);
			return result += ']';
		}

		/* 
		Initializes a new instance of a two dimentional vector from a specified angle.
		Rotates the unit X vector ([1, 0]) the specified amount of radians in the anti-clockwise direction.
		*/
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
		_Check_return_ inline Vector2 Normalize(void)
		{
			return operator/=(Length());
		}

		/* Rotates the vector (anti-clockwise) by a specified amount. */
		_Check_return_ inline Vector2 Rotate(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return *this = Vector2(X * c - Y * s, Y * c + X * s);
		}

		/* Truncates the vector to an integer point. */
		_Check_return_ inline Point Truncate(void) const
		{
			return Point(static_cast<int>(X), static_cast<int>(Y));
		}
	};

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

	/* Gets a value indicating the clockwise direction of the three points, result < 0 if they are clockwise, result > 0 if they are counter clockwise and 0 if they are collinear. */
	_Check_return_ inline float ccw(_In_ Vector2 p1, _In_ Vector2 p2, _In_ Vector2 p3)
	{
		return (p2.X - p1.X) * (p3.Y - p1.Y) - (p2.Y - p1.Y) * (p3.X - p1.X);
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
		return Vector2(max(v.X, w.X), max(v.Y, w.Y));
	}

	/* Gets the lowest value from each component from the specified vectors. */
	_Check_return_ inline Vector2 min(_In_ Vector2 v, _In_ Vector2 w)
	{
		return Vector2(min(v.X, w.X), min(v.Y, w.Y));
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

	/* Gets the reciprocal of the input value. */
	_Check_return_ inline Vector2 recip(_In_ Vector2 v)
	{
		return Vector2(1.0f) / v;
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