#pragma once
#include "Vector3.h"

namespace Plutonium
{
	/* Defines a three dimensional rotation. */
	struct Quaternion
	{
	public:
		/* The first imaginary axis for this quaternion. */
		float I;
		/* The second imaginary axis of this quaternion. */
		float J;
		/* The third imaginary axis of this quaternion. */
		float K;
		/* The real axis of this quaternion. */
		float R;

		/* Initialize a new instance of a quaternion that defines no change in rotation. */
		Quaternion(void)
			: I(0.0f), J(0.0f), K(0.0f), R(1.0f)
		{}

		/* Negates the quaternion. */
		_Check_return_ inline Quaternion operator -(void) const
		{
			return Quaternion(-I, -J, -K, -R);
		}

		/* Subtracts the input quaternion from the quaternion. */
		_Check_return_ inline Quaternion operator -(_In_ Quaternion q) const
		{
			return Quaternion(I - q.I, J - q.J, K - q.K, R - q.R);
		}

		/* Adds the input quaterion to the quaternion. */
		_Check_return_ inline Quaternion operator +(_In_ Quaternion q) const
		{
			return Quaternion(I + q.I, J + q.J, K + q.K, R + q.R);
		}

		/* Multiplies the quaternion by a scalar value. */
		_Check_return_ inline Quaternion operator *(_In_ float v) const
		{
			return Quaternion(I * v, J * v, K * v, R * v);
		}

		/* Multiples the quaterion by the specified quaternion. */
		_Check_return_ Quaternion operator *(_In_ Quaternion q) const;
		/* Divides the quaterion by the specified quaternion. */
		_Check_return_ Quaternion operator /(_In_ Quaternion q) const;

		/* Subtracts the input quaternion from the quaternion. */
		_Check_return_ inline Quaternion operator -=(_In_ Quaternion q)
		{
			I -= q.I;
			J -= q.J;
			K -= q.J;
			R -= q.R;
			return *this;
		}

		/* Adds the input quaternion to the quaternion. */
		_Check_return_ inline Quaternion operator +=(_In_ Quaternion q)
		{
			I += q.I;
			J += q.J;
			K += q.J;
			R += q.R;
			return *this;
		}

		/* Multiplies the quaternion by a scalar value. */
		_Check_return_ inline Quaternion operator *=(_In_ float v)
		{
			I *= v;
			J *= v;
			K *= v;
			R *= v;
			return *this;
		}

		/* Multiplies the quaternion by another quaternion. */
		_Check_return_ inline Quaternion operator *=(_In_ Quaternion q)
		{
			return *this = *this * q;
		}
		/* Divides the quaternion by another quaternion. */
		_Check_return_ inline Quaternion operator /=(_In_ Quaternion q)
		{
			return *this = *this / q;
		}

		/* Checks whether the input quaternion is equal to the quaternion. */
		_Check_return_ inline bool operator ==(_In_ Quaternion q) const
		{
			return I == q.I && J == q.J && K == q.K && R == q.R;
		}

		/* Checks whether the input quaternion differs from the quaternion. */
		_Check_return_ inline bool operator !=(_In_ Quaternion q) const
		{
			return I != q.I || J != q.J || K != q.K || R != q.R;
		}

		/* Gets the magnetude of the quaternion squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return I * I + J * J + K * K + R * R;
		}

		/* Gets the magnetude of the quaternion. */
		_Check_return_ inline float Length(void) const
		{
			return sqrtf(LengthSquared());
		}

		/* Normalizes the current quaternion. */
		inline void Normalize(void)
		{
			operator*=(recip(Length()));
		}

		/* Creates a quaternion rotating around a specified axis. */
		_Check_return_ static Quaternion CreateRotation(_In_ float theta, _In_ Vector3 axis);
		/* Creates a quaternion from euler angles. */
		_Check_return_ static Quaternion CreateRotation(_In_ float yaw, _In_ float pitch, _In_ float roll);
		/* Performs spherical linear interpolation between two quaternions. */
		_Check_return_ static Quaternion SLerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);

	private:
		Quaternion(float i, float j, float k, float r)
			: I(i), J(j), K(k), R(r)
		{}
	};

	/* Calculates the dot product of the two specified quaternions. */
	_Check_return_ inline float dot(_In_ Quaternion q1, _In_ Quaternion q2)
	{
		return q1.I * q2.I + q1.J * q2.J + q1.K * q2.K + q1.R * q2.R;
	}

	/* Gets the normalized version of the specified quaternion. */
	_Check_return_ inline Quaternion normalize(_In_ Quaternion q)
	{
		return q * recip(q.Length());
	}
}