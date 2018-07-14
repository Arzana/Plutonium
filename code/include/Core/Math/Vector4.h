#pragma once
#include "Vector3.h"

namespace Plutonium
{
	/* Defines a four dimentional vector. */
	typedef struct Vector4
	{
		union
		{
			/* All components of the vector. */
			float f[4];

			struct
			{
				/* The X component of the vector. */
				float X;
				/* The Y component of the vector. */
				float Y;
				/* The Z component of the vector. */
				float Z;
				/* The W component of the vector. */
				float W;
			};
		};

		/* Initializes a new instance of a four dimentional  vector with all components set to zero. */
		Vector4(void)
			: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{}

		/* Initializes a new instance of a four dimentional vector with all components set to a specified value. */
		Vector4(_In_ float v)
			: X(v), Y(v), Z(v), W(v)
		{}

		/* Initializes a new instance of a four dimentional vector with all components specified. */
		Vector4(_In_ float x, _In_ float y, _In_ float z, _In_ float w)
			: X(x), Y(y), Z(z), W(w)
		{}

		/* Initializes a new instance of a four dimentional vector with all components specified. */
		Vector4(_In_ Vector3 v, _In_ float w)
			: X(v.X), Y(v.Y), Z(v.Z), W(w)
		{}

		/* Negates the vector. */
		_Check_return_ inline Vector4 operator -(void) const
		{
			return Vector4(-X, -Y, -Z, -W);
		}

		/* Subtracts the input vector from the vector. */
		_Check_return_ inline Vector4 operator -(_In_ Vector4 v) const
		{
			return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W);
		}

		/* Adds the input vector to the vector. */
		_Check_return_ inline Vector4 operator +(_In_ Vector4 v) const
		{
			return Vector4(X + v.X, Y + v.Y, Z + v.Z, W + v.W);
		}

		/* Multiplies the vector by a scalar value. */
		_Check_return_ inline Vector4 operator *(_In_ float v) const
		{
			return Vector4(X * v, Y * v, Z * v, W * v);
		}

		/* Multiples the vector by another vector. */
		_Check_return_ inline Vector4 operator *(_In_ Vector4 v) const
		{
			return Vector4(X * v.X, Y * v.Y, Z * v.Z, W * v.W);
		}

		/* Divides the vector by a scalar value. */
		_Check_return_ inline Vector4 operator /(_In_ float v) const
		{
			return operator*(1.0f / v);
		}

		/* Divides the vector by another vector. */
		_Check_return_ inline Vector4 operator /(_In_ Vector4 v) const
		{
			return Vector4(X / v.X, Y / v.Y, Z / v.Z, W / v.W);
		}

		/* Subtracts the input vector from the vector. */
		inline Vector4 operator -=(_In_ Vector4 v)
		{
			X -= v.X;
			Y -= v.Y;
			Z -= v.Z;
			W -= v.W;
			return *this;
		}

		/* Adds the input vector to the vector. */
		inline Vector4 operator +=(_In_ Vector4 v)
		{
			X += v.X;
			Y += v.Y;
			Z += v.Z;
			W += v.W;
			return *this;
		}

		/* Multiplies the vector by a scalar value. */
		inline Vector4 operator *=(_In_ float v)
		{
			X *= v;
			Y *= v;
			Z *= v;
			W *= v;
			return *this;
		}

		/* Multiplies the vector by another vector. */
		inline Vector4 operator *=(_In_ Vector4 v)
		{
			X *= v.X;
			Y *= v.Y;
			Z *= v.Z;
			W *= v.W;
			return *this;
		}

		/* Divides the vector by a scalar value. */
		inline Vector4 operator /=(_In_ float v)
		{
			return operator*=(1.0f / v);
		}

		/* Divides the vector by another vector. */
		inline Vector4 operator /=(_In_ Vector4 v)
		{
			X /= v.X;
			Y /= v.Y;
			Z /= v.Z;
			W /= v.W;
			return *this;
		}

		/* Checks whether the input vector is equal to the vector. */
		_Check_return_ inline bool operator ==(_In_ Vector4 v) const
		{
			return X == v.X && Y == v.Y && Z == v.Z && W == v.W;
		}

		/* Checks whether the input vector differs from the vector. */
		_Check_return_ inline bool operator !=(_In_ Vector4 v) const
		{
			return X != v.X || Y != v.Y || Z != v.Z || W != v.W;
		}

		/* [0, 0, 0, 0] */
		_Check_return_ static inline Vector4 Zero()
		{
			static Vector4 result = Vector4();
			return result;
		}

		/* [1, 0, 0, 0] */
		_Check_return_ static inline Vector4 UnitX()
		{
			static Vector4 result = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			return result;
		}

		/* [0, 1, 0, 0] */
		_Check_return_ static inline Vector4 UnitY()
		{
			static Vector4 result = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			return result;
		}

		/* [0, 0, 1, 0] */
		_Check_return_ static inline Vector4 UnitZ()
		{
			static Vector4 result = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			return result;
		}

		/* [0, 0, 0, 1] */
		_Check_return_ static inline Vector4 UnitW()
		{
			static Vector4 result = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			return result;
		}

		/* [1, 1, 1, 1] */
		_Check_return_ static inline Vector4 One()
		{
			static Vector4 result = Vector4(1.0f);
			return result;
		}

		/* Gets the magnetude of the vector squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return X * X + Y * Y + Z * Z + W * W;
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
	} Vec4;

	/* Multiplies the vector by a scalar value. */
	_Check_return_ inline Vector4 operator *(_In_ float s, _In_ Vector4 v)
	{
		return v * s;
	}

	/* Gets the absolute value of each vector component. */
	_Check_return_ inline Vector4 abs(_In_ Vector4 v)
	{
		return Vector4(fabsf(v.X), fabsf(v.Y), fabsf(v.Z), fabsf(v.W));
	}

	/* Gets the input vector restricted to the specified range. */
	_Check_return_ inline Vector4 clamp(_In_ Vector4 v, _In_ Vector4 a, _In_ Vector4 b)
	{
		return Vector4(clamp(v.X, a.X, b.X), clamp(v.Y, a.Y, b.Y), clamp(v.Z, a.Z, b.Z), clamp(v.W, a.W, b.W));
	}

	/* Calculates the dot product of the two specified vectors. */
	_Check_return_ inline float dot(_In_ Vector4 v, _In_ Vector4 w)
	{
		return v.X * w.X + v.Y * w.Y + v.Z * w.Z + v.W * w.W;
	}

	/* Gets the highest value from each component from the specified vectors. */
	_Check_return_ inline Vector4 max(_In_ Vector4 v, _In_ Vector4 w)
	{
		return Vector4(__max(v.X, w.X), __max(v.Y, w.Y), __max(v.Z, w.Z), __max(v.W, w.W));
	}

	/* Gets the lowest value from each component from the specified vectors. */
	_Check_return_ inline Vector4 min(_In_ Vector4 v, _In_ Vector4 w)
	{
		return Vector4(__min(v.X, w.X), __min(v.Y, w.Y), __min(v.Z, w.Z), __min(v.W, w.W));
	}

	/* Gets a vector with unit length that has the same direction as the input vector. */
	_Check_return_ inline Vector4 normalize(_In_ Vector4 v)
	{
		return v / v.Length();
	}

	/* Gets the sign of each component of the input vector. */
	_Check_return_ inline Vector4 sign(_In_ Vector4 v)
	{
		return Vector4(sign(v.X), sign(v.Y), sign(v.Z), sign(v.W));
	}

	/* Get the input vector restricted to the specified range in positive and negative direction. */
	_Check_return_ inline Vector4 mclamp(_In_ Vector4 v, _In_ Vector4 a, _In_ Vector4 b)
	{
		return clamp(abs(v), a, b) * sign(v);
	}
}