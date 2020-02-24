#pragma once
#include "Vector3.h"

namespace Pu
{
	/* Defines a four dimensional vector. */
	struct Vector4
	{
		union
		{
			/* All components of the vector. */
			float f[4];

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
				/* The W component of the vector. */
				float W;
			};

			struct
			{
				/* The X, Y and Z component of the vector. */
				Vector3 XYZ;
				/* The W component of the vector. */
				float W;
			};

			struct
			{
				/* The X and Y component of the vector. */
				Vector2 XY;
				/* The Z and W component of the vector. */
				Vector2 ZW;
			};
#pragma warning(pop)
		};

		/* Initializes a new instance of a four dimensional  vector with all components set to zero. */
		Vector4(void)
			: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{}

		/* Initializes a new instance of a four dimensional vector with all components set to a specified value. */
		Vector4(_In_ float v)
			: X(v), Y(v), Z(v), W(v)
		{}

		/* Initializes a new instance of a four dimensional vector with all components specified. */
		Vector4(_In_ float x, _In_ float y, _In_ float z, _In_ float w)
			: X(x), Y(y), Z(z), W(w)
		{}

		/* Initializes a new instance of a four dimensional vector with all components specified. */
		Vector4(_In_ Vector2 xy, _In_ float z, _In_ float w)
			: X(xy.X), Y(xy.Y), Z(z), W(w)
		{}

		/* Initializes a new instance of a four dimensional vector with all components specified. */
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
		_Check_return_ inline Vector4 operator -=(_In_ Vector4 v)
		{
			X -= v.X;
			Y -= v.Y;
			Z -= v.Z;
			W -= v.W;
			return *this;
		}

		/* Adds the input vector to the vector. */
		_Check_return_ inline Vector4 operator +=(_In_ Vector4 v)
		{
			X += v.X;
			Y += v.Y;
			Z += v.Z;
			W += v.W;
			return *this;
		}

		/* Multiplies the vector by a scalar value. */
		_Check_return_ inline Vector4 operator *=(_In_ float v)
		{
			X *= v;
			Y *= v;
			Z *= v;
			W *= v;
			return *this;
		}

		/* Multiplies the vector by another vector. */
		_Check_return_ inline Vector4 operator *=(_In_ Vector4 v)
		{
			X *= v.X;
			Y *= v.Y;
			Z *= v.Z;
			W *= v.W;
			return *this;
		}

		/* Divides the vector by a scalar value. */
		_Check_return_ inline Vector4 operator /=(_In_ float v)
		{
			return operator*=(1.0f / v);
		}

		/* Divides the vector by another vector. */
		_Check_return_ inline Vector4 operator /=(_In_ Vector4 v)
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

		/* Implicitly converts the 4D vector to a string. */
		_Check_return_ inline operator string() const
		{
			string result("[X: ");
			result += string::from(X);
			result += ", Y: ";
			result += string::from(Y);
			result += ", Z: ";
			result += string::from(Z);
			result += ", W: ";
			result += string::from(W);
			return result += ']';
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
		_Check_return_ inline Vector4 Normalize(void)
		{
			return operator/=(Length());
		}
	};

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
		return Vector4(max(v.X, w.X), max(v.Y, w.Y), max(v.Z, w.Z), max(v.W, w.W));
	}

	/* Gets the lowest value from each component from the specified vectors. */
	_Check_return_ inline Vector4 min(_In_ Vector4 v, _In_ Vector4 w)
	{
		return Vector4(min(v.X, w.X), min(v.Y, w.Y), min(v.Z, w.Z), min(v.W, w.W));
	}

	/* Gets a vector with unit length that has the same direction as the input vector. */
	_Check_return_ inline Vector4 normalize(_In_ Vector4 v)
	{
		return v / v.Length();
	}

	/* Checks if two vectors are equal with a specfied error tolerance. */
	_Check_return_ inline bool nrlyeql(_In_ Vector4 v, _In_ Vector4 w, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyeql(v.X, w.X, tolerance) && nrlyeql(v.Y, w.Y, tolerance) && nrlyeql(v.Z, w.Z) && nrlyeql(v.W, w.W);
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