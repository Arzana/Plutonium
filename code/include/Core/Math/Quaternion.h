#pragma once
#include "Vector3.h"

namespace Pu
{
	/* Defines a three dimensional rotation. */
	struct Quaternion
	{
	public:
		/* Defines the first imaginary component of the quaternion. */
		float I;
		/* Defines the second imaginary component of the quaternion. */
		float J;
		/* Defines the third imaginary component of the quaternion. */
		float K;
		/* Defnes the real component of the quaternion. */
		float R;

		/* Initializes a new instance of a quaternion that does nothing. */
		Quaternion(void)
			: R(1.0f), I(0.0f), J(0.0f), K(0.0f)
		{}

		/* Initializes a new instance of a quaternion with specific components. */
		Quaternion(_In_ float r, _In_ float i, _In_ float j, _In_ float k)
			: R(r), I(i), J(j), K(k)
		{}

		/* Negates the quaternion. */
		_Check_return_ inline Quaternion operator -(void) const
		{
			return Quaternion(-R, -I, -J, -K);
		}

		/* Adds the input vector to the quaternion. */
		_Check_return_ inline Quaternion operator +(_In_ Quaternion q) const
		{
			return Quaternion(R + q.R, I + q.I, J + q.J, K + q.K);
		}

		/* Subtracts the input vector from the quaternion. */
		_Check_return_ inline Quaternion operator -(_In_ Quaternion q) const
		{
			return Quaternion(R - q.R, I - q.I, J - q.J, K - q.K);
		}

		/* Multiplies the quaternion by a scalar value. */
		_Check_return_ inline Quaternion operator *(_In_ float v) const
		{
			return Quaternion(R * v, I * v, J * v, K * v);
		}

		/* Divides the quaternion by a scalar value. */
		_Check_return_ inline Quaternion operator /(_In_ float v) const
		{
			return operator*(recip(v));
		}

		/* Gets whether this quaternion should be sorted before the specified quaternion. */
		_Check_return_ inline bool operator <(_In_ Quaternion q) const
		{
			return I < q.I || (!(q.I < I) && J < q.J) || (!(q.I < I) && !(q.J < J) && K < q.K) || (!(q.I < I) && !(q.J < J) && !(q.K < K) && R < q.R);
		}

		/* Multiplies a specified quaternion with the quaternion. */
		_Check_return_ Quaternion operator *(_In_ Quaternion q) const;
		/* Multiplies a specified vector with the quaternion. */
		_Check_return_ Vector3 operator *(_In_ Vector3 v) const;

		/* Adds the specified quaternion to this quaternion. */
		inline Quaternion operator +=(_In_ Quaternion q)
		{
			I += q.I;
			J += q.J;
			K += q.K;
			R += q.R;
			return *this;
		}

		/* Multiplies the quaternion by a scalar value. */
		inline Quaternion operator *=(_In_ float v)
		{
			I *= v;
			J *= v;
			K *= v;
			R *= v;
			return *this;
		}

		/* Multiplies the quaternion with the specified quaternion. */
		inline Quaternion operator *=(_In_ Quaternion q)
		{
			return *this = *this * q;
		}

		/* Divides the quaternion by a scalar value. */
		inline Quaternion operator /=(_In_ float v)
		{
			return operator*=(recip(v));
		}

		/* Checks if two quaternions are equal. */
		_Check_return_ inline bool operator ==(_In_ const Quaternion &q) const
		{
			return R == q.R && I == q.I && J == q.J && K == q.K;
		}
		/* Checks if two quaternions differ. */
		_Check_return_ inline bool operator !=(_In_ const Quaternion &q) const
		{
			return R != q.R || I != q.I || J != q.J || K != q.K;
		}

		/* Gets the magnetude of the quaternion squared. */
		_Check_return_ inline float LengthSquared(void) const
		{
			return sqr(I) + sqr(J) + sqr(K) + sqr(R);
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

		/* Creates a quaternion rotating towards the specified forward direction. */
		_Check_return_ static Quaternion Create(_In_ Vector3 forward, _In_ Vector3 up);
		/* Creates a quaternion rotating around a specified axis. */
		_Check_return_ static Quaternion Create(_In_ float theta, _In_ Vector3 axis);
		/* Creates a quaternion from euler angles. */
		_Check_return_ static Quaternion Create(_In_ float yaw, _In_ float pitch, _In_ float roll);
		/* Gets the rotational delta between two quaternions. */
		_Check_return_ static Quaternion Delta(_In_ Quaternion q1, _In_ Quaternion q2);
		/* Performs nearest neightbor interpolation between two quaternions. */
		_Check_return_ static Quaternion Near(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);
		/* Performs linear interpolation between two quaternions. */
		_Check_return_ static Quaternion Lerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);
		/* Performs spherical linear interpolation between two quaternions. */
		_Check_return_ static Quaternion SLerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ float a);
		/* Performs cubic spherical interpolation between two quaternions and a control quaternion. */
		_Check_return_ static Quaternion CLerp(_In_ Quaternion q1, _In_ Quaternion q2, _In_ Quaternion q3, _In_ float a);
		/* Unpacks the quaterion from the first 63-bits. */
		_Check_return_ static Quaternion Unpack(_In_ int64 packed);
		/* Gets the inverse rotation specified by the quaternion. */
		_Check_return_ Quaternion Inverse(void) const;
		/* Packs the quaterion in 63-bits. */
		_Check_return_ int64 Pack(void) const;
		/* Gets a human readable version of the quaternion. */
		_Check_return_ string ToString(void) const;
	};

	/* Calculates the dot product of the two specified quaternions. */
	_Check_return_ inline float dot(_In_ Quaternion q1, _In_ Quaternion q2)
	{
		return q1.I * q2.I + q1.J * q2.J + q1.K * q2.K + q1.R * q2.R;
	}

	/* Normalizes the specified quaternion. */
	_Check_return_ inline Quaternion normalize(_In_ Quaternion q)
	{
		return q / q.Length();
	}

	/* Checks whether two quaternions are equal within a specified error tolerance. */
	_Check_return_ inline bool nrlyeql(_In_ Quaternion q1, _In_ Quaternion q2, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyeql(q1.I, q2.I, tolerance) && nrlyeql(q1.J, q2.J, tolerance) && nrlyeql(q1.K, q2.K, tolerance) && nrlyeql(q1.R, q2.R, tolerance);
	}

	/* Checks if two quaternions differ within a specific tolerance. */
	_Check_return_ inline bool nrlyneql(_In_ Quaternion q1, _In_ Quaternion q2, _In_opt_ float tolerance = EPSILON)
	{
		return nrlyneql(q1.I, q2.I, tolerance) || nrlyneql(q1.J, q2.J, tolerance) || nrlyneql(q1.K, q2.K, tolerance) || nrlyneql(q1.R, q2.R, tolerance);
	}
}