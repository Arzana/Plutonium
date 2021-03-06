#pragma once
#include "Vector2.h"

namespace Plutonium
{
	/* Defines a three dimentional vector. */
	typedef struct Vector3
	{
		union
		{
			/* All components of the vector. */
			float f[3];

			/* This project is build to compile with the Microsoft compiler and that does allow this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
			struct
			{
				/* The X component of the vector. */
				float X;
				/* The Y component of the vector. */
				float Y;
				/* The Z component of the vector. */
				float Z;
			};

			struct
			{
				/* The X and Y component of the vector. */
				Vector2 XY;
				/* The Z component of the vector. */
				float Z;
			};
#pragma warning(pop)
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
		_Check_return_ inline Vector3 operator -=(_In_ Vector3 v)
		{
			X -= v.X;
			Y -= v.Y;
			Z -= v.Z;
			return *this;
		}

		/* Adds the input vector to the vector. */
		_Check_return_ inline Vector3 operator +=(_In_ Vector3 v)
		{
			X += v.X;
			Y += v.Y;
			Z += v.Z;
			return *this;
		}

		/* Multiplies the vector by a scalar value. */
		_Check_return_ inline Vector3 operator *=(_In_ float v)
		{
			X *= v;
			Y *= v;
			Z *= v;
			return *this;
		}

		/* Multiplies the vector by another vector. */
		_Check_return_ inline Vector3 operator *=(_In_ Vector3 v)
		{
			X *= v.X;
			Y *= v.Y;
			Z *= v.Z;
			return *this;
		}

		/* Divides the vector by a scalar value. */
		_Check_return_ inline Vector3 operator /=(_In_ float v)
		{
			return operator*=(1.0f / v);
		}

		/* Divides the vector by another vector. */
		_Check_return_ inline Vector3 operator /=(_In_ Vector3 v)
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

		/* [0, 0, 0] */
		_Check_return_ static inline Vector3 Zero(void)
		{
			static Vector3 result = Vector3();
			return result;
		}

		/* [1, 0, 0] */
		_Check_return_ static inline Vector3 UnitX(void)
		{
			static Vector3 result = Vector3(1.0f, 0.0f, 0.0f);
			return result;
		}
		/* [0, 1, 0] */
		_Check_return_ static inline Vector3 UnitY(void)
		{
			static Vector3 result = Vector3(0.0f, 1.0f, 0.0f);
			return result;
		}

		/* [0, 0, 1] */
		_Check_return_ static inline Vector3 UnitZ(void)
		{
			static Vector3 result = Vector3(0.0f, 0.0f, 1.0f);
			return result;
		}

		/* [1, 1, 1] */
		_Check_return_ static inline Vector3 One(void)
		{
			static Vector3 result = Vector3(1.0f);
			return result;
		}

		/* [1, 0, 0] */
		_Check_return_ static inline Vector3 Right(void)
		{
			return UnitX();
		}

		/* [-1, 0, 0] */
		_Check_return_ static inline Vector3 Left(void)
		{
			static Vector3 result = Vector3(-1.0f, 0.0f, 0.0f);
			return result;
		}

		/* [0, 1, 0] */
		_Check_return_ static inline Vector3 Up(void)
		{
			return UnitY();
		}

		/* [0, -1, 0] */
		_Check_return_ static inline Vector3 Down(void)
		{
			static Vector3 result = Vector3(0.0f, -1.0f, 0.0f);
			return result;
		}

		/* [0, 0, 1] */
		_Check_return_ static inline Vector3 Backward(void)
		{
			return UnitZ();
		}

		/* [0, 0, -1] */
		_Check_return_ static inline Vector3 Forward(void)
		{
			static Vector3 result = Vector3(0.0f, 0.0f, -1.0f);
			return result;
		}

		/* Creates a directional vector from an angle around the Y axis. */
		_Check_return_ static inline Vector3 FromYaw(_In_ float theta)
		{
			return Vector3(cosf(theta), 0.0f, sinf(theta));
		}

		/* Creates a directional vector from an angle around the X axis. */
		_Check_return_ static inline Vector3 FromPitch(_In_ float theta)
		{
			return Vector3(0.0f, cosf(theta), sinf(theta));
		}

		/* Creates a directional vector from an angle around the Z axis. */
		_Check_return_ static inline Vector3 FromRoll(_In_ float theta)
		{
			return Vector3(cosf(theta), sinf(theta), 0.0f);
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

		/* Normalizes the current vector. */
		inline void Normalize(void)
		{
			operator/=(Length());
		}
	} Vec3;

	/* Multiplies the vector by a scalar value. */
	_Check_return_ inline Vector3 operator *(_In_ float s, _In_ Vector3 v)
	{
		return v * s;
	}

	/* Divides the vector by a scalar value. */
	_Check_return_ inline Vector3 operator /(_In_ float s, _In_ Vector3 v)
	{
		return v / s;
	}

	/* Gets the absolute value of each vector component. */
	_Check_return_ inline Vector3 abs(_In_ Vector3 v)
	{
		return Vector3(fabsf(v.X), fabsf(v.Y), fabsf(v.Z));
	}

	/* Gets the input vector restricted to the specified range. */
	_Check_return_ inline Vector3 clamp(_In_ Vector3 v, _In_ Vector3 a, _In_ Vector3 b)
	{
		return Vector3(clamp(v.X, a.X, b.X), clamp(v.Y, a.Y, b.Y), clamp(v.Z, a.Z, b.Z));
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

	/* Gets the squared distance between the two specified vectors. */
	_Check_return_ inline float sqrdist(_In_ Vector3 v, _In_ Vector3 w)
	{
		return (w - v).LengthSquared();
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

	/* Checks if two vectors are equal with a specfied error tolerance. */
	_Check_return_ inline bool nrlyeql(_In_ Vector3 v, _In_ Vector3 w, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyeql(v.X, w.X, tolerance) && nrlyeql(v.Y, w.Y, tolerance) && nrlyeql(v.Z, w.Z);
	}

	/* Gets the direction from a to b. */
	_Check_return_ inline Vector3 dir(_In_ Vector3 a, _In_ Vector3 b)
	{
		return normalize(b - a);
	}

	/* Calculates the reflection vector of the specified input vector around the specified normal. */
	_Check_return_ inline Vector3 reflect(_In_ Vector3 v, _In_ Vector3 n)
	{
		return v - 2.0f * dot(v, n) * n;
	}

	/* Gets the sign of each component of the input vector. */
	_Check_return_ inline Vector3 sign(_In_ Vector3 v)
	{
		return Vector3(sign(v.X), sign(v.Y), sign(v.Z));
	}

	/* Get the input vector restricted to the specified range in positive and negative direction. */
	_Check_return_ inline Vector3 mclamp(_In_ Vector3 v, _In_ Vector3 a, _In_ Vector3 b)
	{
		return clamp(abs(v), a, b) * sign(v);
	}
}