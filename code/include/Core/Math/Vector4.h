#pragma once
#include <cmath>
#include <sal.h>

namespace Plutonium
{
	/* Defines a four dimentional vector. */
	typedef struct Vector4
	{
		/* [0, 0, 0, 0] */
		const static Vector4 Zero;
		/* [1, 0, 0, 0] */
		const static Vector4 UnitX;
		/* [0, 1, 0, 0] */
		const static Vector4 UnitY;
		/* [0, 0, 1, 0] */
		const static Vector4 UnitZ;
		/* [0, 0, 0, 1] */
		const static Vector4 UnitW;
		/* [1, 1, 1, 1] */
		const static Vector4 One;

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
	} Vec4;

	/* Multiplies the vector by a scalar value. */
	_Check_return_ inline Vector4 operator *(_In_ float s, _In_ Vector4 v)
	{
		return v * s;
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
}