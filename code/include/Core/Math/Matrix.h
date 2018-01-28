#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace Plutonium
{
	/* Defines a 4x4 square matrix. */
	struct Matrix
	{
	public:
		/* Initializes a new instance of a 4x4 square matrix as an identity matrix. */
		Matrix(void)
			: c1(1.0f, 0.0f, 0.0f, 0.0f)
			, c2(0.0f, 1.0f, 0.0f, 0.0f)
			, c3(0.0f, 0.0f, 1.0f, 0.0f)
			, c4(0.0f, 0.0f, 0.0f, 1.0f)
		{}

		/* Transforms the specified vector by the matrix. */
		_Check_return_ inline Vector3 operator *(_In_ Vector3 v) const
		{
			return v.X * GetRight() + v.Y * GetUp() + v.Z * GetBackward() + GetTranslation();
		}

		/* Transforms the specified vector by the matrix. */
		_Check_return_ inline Vector4 operator *(_In_ Vector4 v) const
		{
			return v.X * c1 + v.Y * c2 + v.Z * c3 + v.W * c4;
		}

		/* Adds a specified matrix to the matrix. */
		_Check_return_ Matrix operator +(_In_ const Matrix &m) const;
		/* Subtracts a specified matrix from the matrix. */
		_Check_return_ Matrix operator -(_In_ const Matrix &m) const;
		/* Multiplies a specified matrix with the matrix. */
		_Check_return_ Matrix operator *(_In_ const Matrix &m) const;
		/* Multiplies a specified matrix with the matrix. */
		inline Matrix operator *=(_In_ const Matrix &m)
		{
			return *this = *this * m;
		}

		/* Creates a translation matrix from three individual components. */
		_Check_return_ static inline Matrix CreateTranslation(_In_ float x, _In_ float y, _In_ float z)
		{
			return Matrix(1.0f, 0.0f, 0.0f, x, 0.0f, 1.0f, 0.0f, y, 0.0f, 0.0f, 1.0f, z, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a translation matrix from a vector. */
		_Check_return_ static inline Matrix CreateTranslation(_In_ Vector3 v)
		{
			return Matrix(1.0f, 0.0f, 0.0f, v.X, 0.0f, 1.0f, 0.0f, v.Y, 0.0f, 0.0f, 1.0f, v.Z, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a scalar matrix from a scalar value. */
		_Check_return_ static inline Matrix CreateScalar(_In_ float v)
		{
			return Matrix(v, 0.0f, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 0.0f, 0.0f, v, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a scalar matrix from three individual components. */
		_Check_return_ static inline Matrix CreateScalar(_In_ float x, _In_ float y, _In_ float z)
		{
			return Matrix(x, 0.0f, 0.0f, 0.0f, 0.0f, y, 0.0f, 0.0f, 0.0f, 0.0f, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a scalar matrix from a vector. */
		_Check_return_ static inline Matrix CreateScalar(_In_ Vector3 v)
		{
			return Matrix(v.X, 0.0f, 0.0f, 0.0f, 0.0f, v.Y, 0.0f, 0.0f, 0.0f, 0.0f, v.Z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a rotation matrix around a specified axis. */
		_Check_return_ static Matrix CreateRotation(_In_ float theta, _In_ Vector3 axis);
		/* Creates a rotation matrix from euler angles. */
		_Check_return_ static Matrix CreateRotation(_In_ float yaw, _In_ float pitch, _In_ float roll);

		/* Creates a rotation matrix around the X axis with an angle of theta. */
		_Check_return_ static inline Matrix CreateRotationX(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a rotation matrix around the Y axis with an angle of theta. */
		_Check_return_ static inline Matrix CreateRotationY(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(c, 0.0f, s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a rotation matrix around the Z axis with an angle of theta. */
		_Check_return_ static inline Matrix CreateRotationZ(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(c, -s, 0.0f, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates an orthographics projection matrix. */
		_Check_return_ static Matrix CreateOrtho(_In_ float left, _In_ float right, _In_ float bottom, _In_ float top, _In_ float near, _In_ float far);
		/* Creates a frustum projection matrix. */
		_Check_return_ static Matrix CreateFrustum(_In_ float left, _In_ float right, _In_ float bottom, _In_ float top, _In_ float near, _In_ float far);
		/* Creates a perspective projection matrix. */
		_Check_return_ static Matrix CreatPerspective(_In_ float fovY, _In_ float aspr, _In_ float near, _In_ float far);
		/* Creates a look at view matrix. */
		_Check_return_ static Matrix CreateLookAt(_In_ Vector3 pos, _In_ Vector3 target, _In_ Vector3 up);

		/* Gets the translation vector of the matrix. */
		_Check_return_ inline Vector3 GetTranslation(void) const
		{
			return Vector3(c4.X, c4.Y, c4.Z);
		}

		/* Gets the right vector of the matrix. */
		_Check_return_ inline Vector3 GetRight(void) const
		{
			return Vector3(c1.X, c1.Y, c1.Z);
		}

		/* Gets the left vector of the matrix. */
		_Check_return_ inline Vector3 GetLeft(void) const
		{
			return -GetRight();
		}

		/* Gets the up vector of the matrix. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return Vector3(c2.X, c2.Y, c2.Z);
		}

		/* Gets the down vector of the matrix. */
		_Check_return_ inline Vector3 GetDown(void) const
		{
			return -GetUp();
		}

		/* Gets the backward vector of the matrix. */
		_Check_return_ inline Vector3 GetBackward(void) const
		{
			return Vector3(c3.X, c3.Y, c3.Z);
		}

		/* Gets the forward vector of the matrix. */
		_Check_return_ inline Vector3 GetForward(void) const
		{
			return -GetBackward();
		}

		/* Gets the underlying components of the matrix. */
		_Check_return_ inline const float* GetComponents(void) const
		{
			return f;
		}

		/* Calculates the determinant of the matrix. */
		_Check_return_ float GetDeterminant(void) const;
		/* Calculates the inverse of the matrix. */
		_Check_return_ Matrix GetInverse(void) const;
		/* Gets the transpose of the matrix. */
		_Check_return_ Matrix GetTranspose(void) const;

		/* Sets the translation vector of the matrix. */
		inline void SetTranslation(_In_ Vector3 v)
		{
			c4 = Vector4(v.X, v.Y, v.Z, c4.W);
		}

		/* Sets the orientation of the matrix. */
		void SetOrientation(_In_ float yaw, _In_ float pitch, _In_ float roll);
		/* Sets the scale of the matrix. */
		void SetScale(_In_ float v);
		/* Sets the scale of the matrix. */
		void SetScale(_In_ float x, _In_ float y, _In_ float z);
		/* Sets the scale of the matrix. */
		void SetScale(_In_ Vector3 v);

	private:
		union
		{
			float f[16];

			struct
			{
				Vector4 c1, c2, c3, c4;
			};
		};

		Matrix(const Vector4 &c1, const Vector4 &c2, const Vector4 &c3, const Vector4 &c4)
			: c1(c1), c2(c2), c3(c3), c4(c4)
		{}

		Matrix(float m00, float m01, float m02, float m03,
			   float m10, float m11, float m12, float m13,
			   float m20, float m21, float m22, float m23,
			   float m30, float m31, float m32, float m33)
			: c1(m00, m10, m20, m30), c2(m01, m11, m21, m31), c3(m02, m12, m22, m32), c4(m03, m13, m23, m33)
		{}
	};
}