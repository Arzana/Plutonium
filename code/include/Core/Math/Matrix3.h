#pragma once
#include "Quaternion.h"

namespace Pu
{
	/* Defines a 3x3 column-major square matrix. */
	struct Matrix3
	{
	public:
		/* Initializes a new instanace of a 3x3 square matrix as an identity matrix. */
		Matrix3(void)
			: c1(1.0f, 0.0f, 0.0f)
			, c2(0.0f, 1.0f, 0.0f)
			, c3(0.0f, 0.0f, 1.0f)
		{}

		/* Initializes a new instance of a 3x3 square matrix with all components specified. */
		Matrix3(float m00, float m01, float m02,
			float m10, float m11, float m12,
			float m20, float m21, float m22)
			: c1(m00, m10, m20), c2(m01, m11, m21), c3(m02, m12, m22)
		{}

		/* Transforms the specified vector by the matrix. */
		_Check_return_ inline Vector3 operator *(_In_ Vector3 v) const
		{
			return v.X * c1 + v.Y * c2 * v.Z * c3;
		}

		/* Adds a specified matrix to the matrix. */
		_Check_return_ inline Matrix3 operator +(_In_ const Matrix3 &m) const
		{
			return Matrix3(c1 + m.c1, c2 + m.c2, c3 + m.c3);
		}

		/* Subtracts a specified matrix from the matrix. */
		_Check_return_ inline Matrix3 operator -(_In_ const Matrix3 &m) const
		{
			return Matrix3(c1 - m.c1, c2 - m.c2, c3 - m.c3);
		}

		/* Multiplies a specified matrix with the matrix. */
		_Check_return_ inline Matrix3 operator *(_In_ const Matrix3 &m) const
		{
			return Matrix3(operator*(m.c1), operator*(m.c2), operator*(m.c3));
		}

		/* Scales the matrix by a specified amount. */
		_Check_return_ inline Matrix3 operator *(_In_ float scalar) const
		{
			return Matrix3(c1 * scalar, c2 * scalar, c3 * scalar);
		}

		/* Multiplies a specified matrix with the matrix. */
		_Check_return_ inline Matrix3 operator *=(_In_ const Matrix3 &m)
		{
			return *this = *this * m;
		}

		/* Creates a scalar matrix from a scalar value. */
		_Check_return_ static inline Matrix3 CreateScalar(_In_ float v)
		{
			return Matrix3(v, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 0.0f, v);
		}

		/* Creates a scalar matrix from three individual components. */
		_Check_return_ static inline Matrix3 CreateScalar(_In_ float x, _In_ float y, _In_ float z)
		{
			return Matrix3(x, 0.0f, 0.0f, 0.0f, y, 0.0f, 0.0f, 0.0f, z);
		}

		/* Creates a scalar matrix from a vector. */
		_Check_return_ static inline Matrix3 CreateScalar(_In_ Vector3 v)
		{
			return Matrix3(v.X, 0.0f, 0.0f, 0.0f, v.Y, 0.0f, 0.0f, 0.0f, v.Z);
		}

		/* Creates a rotation matrix around a specified axis. */
		_Check_return_ static Matrix3 CreateRotation(_In_ float theta, _In_ Vector3 axis);
		/* Creates a rotation matrix from a quaternion. */
		_Check_return_ static Matrix3 CreateRotation(_In_ Quaternion quaternion);

		/* Creates a rotation matrix from euler angles. */
		_Check_return_ static Matrix3 CreateRotation(_In_ float pitch, _In_ float yaw, _In_ float roll)
		{
			return CreateRotation(Quaternion::Create(pitch, yaw, roll));
		}

		/* Creates a rotation matrix around the X axis with an angle of theta. */
		_Check_return_ static inline Matrix3 CreatePitch(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix3(1.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, s, c);
		}

		/* Creates a rotation matrix around the Y axis with an angle of theta. */
		_Check_return_ static inline Matrix3 CreateYaw(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix3(c, 0.0f, s, 0.0f, 1.0f, 0.0f, -s, 0.0f, c);
		}

		/* Creates a rotation matrix around the Z axis with an angle of theta. */
		_Check_return_ static inline Matrix3 CreateRoll(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix3(c, -s, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Gets the right vector of the matrix. */
		_Check_return_ inline Vector3 GetRight(void) const
		{
			return c1;
		}

		/* Gets the left vector of the matrix. */
		_Check_return_ inline Vector3 GetLeft(void) const
		{
			return -GetRight();
		}

		/* Gets the up vector of the matrix. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return c2;
		}

		/* Gets the down vector of the matrix. */
		_Check_return_ inline Vector3 GetDown(void) const
		{
			return -GetUp();
		}

		/* Gets the forward vector of the matrix. */
		_Check_return_ inline Vector3 GetForward(void) const
		{
			return c3;
		}

		/* Gets the backward vector of the matrix. */
		_Check_return_ inline Vector3 GetBackward(void) const
		{
			return -GetForward();
		}

		/* Gets the underlying components of the matrix. */
		_Check_return_ inline const float* GetComponents(void) const
		{
			return f;
		}

		/* Calculates the determinant of the matrix. */
		_Check_return_ float GetDeterminant(void) const;
		/* Calculates the inverse of the matrix. */
		_Check_return_ Matrix3 GetInverse(void) const;
		/* Gets the transpose of the matrix. */
		_Check_return_ Matrix3 GetTranspose(void) const;
		/* Gets a human readable version of this 3x3 matrix. */
		_Check_return_ string ToString(void) const;

	private:
		friend struct Matrix;

		union
		{
			float f[9];

			/* This project is build to compile with the Microsoft compiler which allows this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
			struct
			{
				Vector3 c1, c2, c3;
			};
#pragma warning(pop)
		};

		Matrix3(const Vector3 &c1, const Vector3 &c2, const Vector3 &c3)
			: c1(c1), c2(c2), c3(c3)
		{}
	};
}