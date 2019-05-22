#pragma once
#include "Vector3.h"

namespace Pu
{
	/* Defines a three dimensional rotation. */
	struct Quaternion
	{
	public:
		/* Initializes a new instance of a quaternion that does nothing. */
		Quaternion(void)
			: r(1.0f), i(0.0f), j(0.0f), k(0.0f)
		{}

		/* Initializes a new instance of a quaternion with specific components. */
		Quaternion(_In_ float r, _In_ float i, _In_ float j, _In_ float k)
			: r(r), i(i), j(j), k(k)
		{}

		/* Adds the input vector to the quaternion. */
		_Check_return_ inline Quaternion operator +(_In_ Quaternion q) const
		{
			return Quaternion(r + q.r, i + q.i, j + q.j, k + q.k);
		}

		/* Subtracts the input vector from the quaternion. */
		_Check_return_ inline Quaternion operator -(_In_ Quaternion q) const
		{
			return Quaternion(r - q.r, i - q.i, j - q.j, k - q.k);
		}

		/* Multiplies the quaternion by a scalar value. */
		_Check_return_ inline Quaternion operator *(_In_ float v) const
		{
			return Quaternion(r * v, i * v, j * v, k * v);
		}

		/* Multiplies a specified quaternion with the quaternion. */
		_Check_return_ Quaternion operator *(_In_ const Quaternion &q) const;

		/* Multiplies the quaternion by a scalar value. */
		inline Quaternion operator *=(_In_ float v)
		{
			i *= v;
			j *= v;
			k *= v;
			r *= v;
			return *this;
		}

		/* Divides the quaternion by a scalar value. */
		inline Quaternion operator /=(_In_ float v)
		{
			return operator*=(1.0f / v);
		}

		/* Checks if two quaternions are equal. */
		_Check_return_ inline bool operator ==(_In_ const Quaternion &q) const
		{
			return r == q.r && i == q.i && j == q.j && k == q.k;
		}
		/* Checks if two quaternions differ. */
		_Check_return_ inline bool operator !=(_In_ const Quaternion &q) const
		{
			return r != q.r || i != q.i || j != q.j || k != q.k;
		}

		/* Implicitly converts the quaternion to a string. */
		_Check_return_ inline operator string() const
		{
			string result("[I: ");
			result += string::from(i);
			result += ", J: ";
			result += string::from(j);
			result += ", K: ";
			result += string::from(k);
			result += ", R: ";
			result += string::from(r);
			return result += ']';
		}

		/* Gets the magnetude of the quaternion squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return i * i + j * j + k * k + r * r;
		}

		/* Gets the magnetude of the quaternion. */
		_Check_return_ inline float Length(void) const
		{
			return sqrtf(LengthSquared());
		}

		/* Normalizes the current quaternion. */
		_Check_return_ inline Quaternion Normalize(void)
		{
			return operator/=(Length());
		}

		/* Creates a quaternion rotating around a specified axis. */
		_Check_return_ static Quaternion CreateRotation(_In_ float theta, _In_ Vector3 axis);
		/* Creates a quaternion from euler angles. */
		_Check_return_ static Quaternion CreateRotation(_In_ float yaw, _In_ float pitch, _In_ float roll);
		/* Performs nearest neightbor interpolation between two quaternions. */
		_Check_return_ static Quaternion Near(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);
		/* Performs linear interpolation between two quaternions. */
		_Check_return_ static Quaternion Lerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);
		/* Performs spherical linear interpolation between two quaternions. */
		_Check_return_ static Quaternion SLerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);

	private:
		friend struct Matrix;
		friend class BinaryWriter;
		friend float dot(Quaternion, Quaternion);

		float i, j, k, r;
	};

	/* Calculates the dot product of the two specified quaternions. */
	_Check_return_ inline float dot(_In_ Quaternion q1, _In_ Quaternion q2)
	{
		return q1.i * q2.i + q1.j * q2.j + q1.k * q2.k + q1.r * q2.r;
	}
}