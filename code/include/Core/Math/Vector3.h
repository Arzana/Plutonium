#pragma once
#include <cmath>
#include <sal.h>

namespace Plutonium
{
	/* Defines a three dimentional vector. */
	typedef struct Vector3
	{
		/* [0, 0, 0] */
		const static Vector3 Zero;
		/* [1, 0, 0] */
		const static Vector3 UnitX;
		/* [0, 1, 0] */
		const static Vector3 UnitY;
		/* [0, 0, 1] */
		const static Vector3 UnitZ;
		/* [1, 1, 1] */
		const static Vector3 One;
		/* [1, 0, 0] */
		const static Vector3 Right;
		/* [-1, 0, 0] */
		const static Vector3 Left;
		/* [0, 1, 0] */
		const static Vector3 Up;
		/* [0, -1, 0] */
		const static Vector3 Down;
		/* [0, 0, 1] */
		const static Vector3 Backward;
		/* [0, 0, -1] */
		const static Vector3 Forward;

		union
		{
			/* All components of the vector. */
			float f[3];

			struct
			{
				/* The X component of the vector. */
				float X;
				/* The Y component of the vector. */
				float Y;
				/* The Z component of the vector. */
				float Z;
			};
		};

		/* Initializes a new instance of a three dimentional  vector with all components set to zero. */
		Vector3(void)
			: X(0.0f), Y(0.0f), Z(0.0f)
		{}

		/* Initializes a new instance of a three dimentional vector with all components set to a specified value. */
		Vector3(_In_ float v)
			: X(v), Y(v), Z(v)
		{}

		/* Initializes a new instance of a three dimentional vector with all components specified. */
		Vector3(_In_ float x, _In_ float y, _In_ float z)
			: X(x), Y(y), Z(z)
		{}

		/* Negates the vector. */
		_Check_return_ inline Vector3 operator -(void) const
		{
			return Vector3(-X, -Y, -Z);
		}

		/* Subtracts the input vector from the vector. */
		_Check_return_ inline Vector3 operator -(_In_ Vector3 v) const
		{
			return Vector3(X - v.X, Y - v.Y, Z - v.Z);
		}

		/* Adds the input vector to the vector. */
		_Check_return_ inline Vector3 operator +(_In_ Vector3 v) const
		{
			return Vector3(X + v.X, Y + v.Y, Z + v.Z);
		}

		/* Multiplies the vector by a scalar value. */
		_Check_return_ inline Vector3 operator *(_In_ float v) const
		{
			return Vector3(X * v, Y * v, Z * v);
		}

		/* Multiples the vector by another vector. */
		_Check_return_ inline Vector3 operator *(_In_ Vector3 v) const
		{
			return Vector3(X * v.X, Y * v.Y, Z * v.Z);
		}

		/* Divides the vector by a scalar value. */
		_Check_return_ inline Vector3 operator /(_In_ float v) const
		{
			return operator*(1.0f / v);
		}

		/* Divides the vector by another vector. */
		_Check_return_ inline Vector3 operator /(_In_ Vector3 v) const
		{
			return Vector3(X / v.X, Y / v.Y, Z / v.Z);
		}

		/* Subtracts the input vector from the vector. */
		inline Vector3 operator -=(_In_ Vector3 v)
		{
			X -= v.X;
			Y -= v.Y;
			Z -= v.Z;
			return *this;
		}

		/* Adds the input vector to the vector. */
		inline Vector3 operator +=(_In_ Vector3 v)
		{
			X += v.X;
			Y += v.Y;
			Z += v.Z;
			return *this;
		}

		/* Multiplies the vector by a scalar value. */
		inline Vector3 operator *=(_In_ float v)
		{
			X *= v;
			Y *= v;
			Z *= v;
			return *this;
		}

		/* Multiplies the vector by another vector. */
		inline Vector3 operator *=(_In_ Vector3 v)
		{
			X *= v.X;
			Y *= v.Y;
			Z *= v.Z;
			return *this;
		}

		/* Divides the vector by a scalar value. */
		inline Vector3 operator /=(_In_ float v)
		{
			return operator*=(1.0f / v);
		}

		/* Divides the vector by another vector. */
		inline Vector3 operator /=(_In_ Vector3 v)
		{
			X /= v.X;
			Y /= v.Y;
			Z /= v.Z;
			return *this;
		}

		/* Checks whether the input vector is equal to the vector. */
		_Check_return_ inline bool operator ==(_In_ Vector3 v) const
		{
			return X == v.X && Y == v.Y && Z == v.Z;
		}

		/* Checks whether the input vector differs from the vector. */
		_Check_return_ inline bool operator !=(_In_ Vector3 v) const
		{
			return X != v.X || Y != v.Y || Z != v.Z;
		}

		/* Gets the magnetude of the vector squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return X * X + Y * Y + Z * Z;
		}

		/* Gets the magnetude of the vector. */
		_Check_return_ inline float Length(void) const
		{
			return sqrtf(LengthSquared());
		}
	} Vec3;

	/* Multiplies the vector by a scalar value. */
	_Check_return_ inline Vector3 operator *(_In_ float s, _In_ Vector3 v)
	{
		return v * s;
	}

	/* Calculates the cross product between two vectors. */
	_Check_return_ inline Vector3 cross(_In_ Vector3 v, _In_ Vector3 w)
	{
		const float x = v.Y * w.Z - v.Z * w.Y;
		const float y = v.Z * w.X - v.X * w.Z;
		const float z = v.X * w.Y - v.Y * w.X;
		return Vector3(x, y, z);
	}

	/* Gets the distance between the two specified vectors. */
	_Check_return_ inline float dist(_In_ Vector3 v, _In_ Vector3 w)
	{
		return (w - v).Length();
	}

	/* Calculates the dot product of the two specified vectors. */
	_Check_return_ inline float dot(_In_ Vector3 v, _In_ Vector3 w)
	{
		return v.X * w.X + v.Y * w.Y + v.Z * w.Z;
	}

	/* Gets the highest value from each component from the specified vectors. */
	_Check_return_ inline Vector3 max(_In_ Vector3 v, _In_ Vector3 w)
	{
		return Vector3(__max(v.X, w.X), __max(v.Y, w.Y), __max(v.Z, w.Z));
	}

	/* Gets the lowest value from each component from the specified vectors. */
	_Check_return_ inline Vector3 min(_In_ Vector3 v, _In_ Vector3 w)
	{
		return Vector3(__min(v.X, w.X), __min(v.Y, w.Y), __min(v.Z, w.Z));
	}

	/* Gets a vector with unit length that has the same direction as the input vector. */
	_Check_return_ inline Vector3 normalize(_In_ Vector3 v)
	{
		return v / v.Length();
	}

	/* Calculates the reflection vector of the specified input vector around the specified normal. */
	_Check_return_ inline Vector3 reflect(_In_ Vector3 v, _In_ Vector3 n)
	{
		return v - 2.0f * dot(v, n) * n;
	}
}