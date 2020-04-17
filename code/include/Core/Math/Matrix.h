#pragma once
#include "Vector4.h"
#include "Quaternion.h"

#if defined(near)
#undef near
#endif
#if defined(far)
#undef far
#endif

namespace Pu
{
	/* Defines a 4x4 column-major square matrix. */
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

		/* Transforms the specified vector by the matrix (threats it as a point). */
		_Check_return_ inline Vector3 operator *(_In_ Vector3 v) const
		{
			return v.X * c1.XYZ + v.Y * c2.XYZ + v.Z * c3.XYZ + c4.XYZ;
		}

		/* Transforms the specified vector by the matrix. */
		_Check_return_ inline Vector4 operator *(_In_ Vector4 v) const
		{
			return v.X * c1 + v.Y * c2 + v.Z * c3 + v.W * c4;
		}

		/* Adds a specified matrix to the matrix. */
		_Check_return_ inline Matrix operator +(_In_ const Matrix &m) const
		{
			return Matrix(c1 + m.c1, c2 + m.c2, c3 + m.c3, c4 + m.c4);
		}
		/* Subtracts a specified matrix from the matrix. */
		_Check_return_ inline Matrix operator -(_In_ const Matrix &m) const
		{
			return Matrix(c1 - m.c1, c2 - m.c2, c3 - m.c3, c4 - m.c4);
		}
		/* Multiplies a specified matrix with the matrix. */
		_Check_return_ inline Matrix operator *(_In_ const Matrix &m) const
		{
			return Matrix(operator*(m.c1), operator*(m.c2), operator*(m.c3), operator*(m.c4));
		}
		/* Scales the matrix by a specified amount. */
		_Check_return_ inline Matrix operator *(_In_ float scalar) const
		{
			return Matrix(c1 * scalar, c2 * scalar, c3 * scalar, c4 * scalar);
		}
		/* Multiplies a specified matrix with the matrix. */
		_Check_return_ inline Matrix operator *=(_In_ const Matrix &m)
		{
			return *this = *this * m;
		}

		/* Checks if two matrices are equal. */
		_Check_return_ inline bool operator ==(_In_ const Matrix &m) const
		{
			return c1 == m.c1 && c2 == m.c2 && c3 == m.c3 && c4 == m.c4;
		}
		/* Checks if two matrices differ. */
		_Check_return_ inline bool operator !=(_In_ const Matrix &m) const
		{
			return c1 != m.c1 || c2 != m.c2 || c3 != m.c3 || c4 != m.c4;
		}

		/* Implicitly converts the 4x4 matrix to a string. */
		_Check_return_ operator string() const;

		/* Creates a translation matrix from two individual components. */
		_Check_return_ static inline Matrix CreateTranslation(_In_ Vector2 v)
		{
			return Matrix(1.0f, 0.0f, 0.0f, v.X, 0.0f, 1.0f, 0.0f, v.Y, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
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

		/* Creates a matrix with all components set to zero. */
		_Check_return_ static inline Matrix CreateZero(void)
		{
			return Matrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}

		/* Creates a scale and translation matrix. */
		_Check_return_ static Matrix CreateScaledTranslation(_In_ Vector3 translation, _In_ float scalar);
		/* Creates a rotation matrix around a specified axis. */
		_Check_return_ static Matrix CreateRotation(_In_ float theta, _In_ Vector3 axis);
		/* Creates a rotation matrix from a quaternion. */
		_Check_return_ static Matrix CreateRotation(_In_ Quaternion quaternion);

		/* Creates a rotation matrix from euler angles. */
		_Check_return_ static Matrix CreateRotation(_In_ float yaw, _In_ float pitch, _In_ float roll)
		{
			return CreateRotation(Quaternion::Create(yaw, pitch, roll));
		}

		/* Creates a rotation matrix around the X axis with an angle of theta. */
		_Check_return_ static inline Matrix CreatePitch(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, c, -s, 0.0f, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a rotation matrix around the Y axis with an angle of theta. */
		_Check_return_ static inline Matrix CreateYaw(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(c, 0.0f, s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -s, 0.0f, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a rotation matrix around the Z axis with an angle of theta. */
		_Check_return_ static inline Matrix CreateRoll(_In_ float theta)
		{
			const float c = cosf(theta);
			const float s = sinf(theta);
			return Matrix(c, -s, 0.0f, 0.0f, s, c, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		}

		/* Creates a 2D world matrix from the specified parameters. */
		_Check_return_ static inline Matrix CreateWorld(_In_ Vector2 pos, _In_ float theta, _In_ Vector2 scale)
		{
			return Matrix::CreateTranslation(pos.X, pos.Y, 0.0f) * Matrix::CreateRoll(theta) * Matrix::CreateScalar(scale.X, scale.Y, 1.0f);
		}

		/* Creates a 3D world matrix from the specified parameters. */
		_Check_return_ static inline Matrix CreateWorld(_In_ Vector3 pos, _In_ Quaternion orientation, _In_ Vector3 scale)
		{
			return Matrix::CreateTranslation(pos) * Matrix::CreateRotation(orientation) * Matrix::CreateScalar(scale);
		}

		/* Creates an orthographics projection matrix (assumes left and bottom are at [0,0]). */
		_Check_return_ static Matrix CreateOrtho(_In_ float width, _In_ float height, _In_ float near, _In_ float far);
		/* Creates an orthographics projection matrix. */
		_Check_return_ static Matrix CreateOrtho(_In_ float left, _In_ float right, _In_ float bottom, _In_ float top, _In_ float near, _In_ float far);
		/* Creates a frustum projection matrix. */
		_Check_return_ static Matrix CreateFrustum(_In_ float left, _In_ float right, _In_ float bottom, _In_ float top, _In_ float near, _In_ float far);
		/* Creates a perspective projection matrix. */
		_Check_return_ static Matrix CreatePerspective(_In_ float fovY, _In_ float aspr, _In_ float near, _In_ float far);
		/* Creates a look in view matrix. */
		_Check_return_ static Matrix CreateLookIn(_In_ Vector3 pos, _In_ Vector3 direction, _In_ Vector3 up);

		/* Creates a look at view matrix. */
		_Check_return_ static Matrix CreateLookAt(_In_ Vector3 pos, _In_ Vector3 target, _In_ Vector3 up)
		{
			return CreateLookIn(pos, dir(pos, target), up);
		}

		/* Gets the translation vector of the matrix. */
		_Check_return_ inline Vector3 GetTranslation(void) const
		{
			return c4.XYZ;
		}

		/* Gets the right vector of the matrix. */
		_Check_return_ inline Vector3 GetRight(void) const
		{
			return c1.XYZ;
		}

		/* Gets the left vector of the matrix. */
		_Check_return_ inline Vector3 GetLeft(void) const
		{
			return -GetRight();
		}

		/* Gets the up vector of the matrix. */
		_Check_return_ inline Vector3 GetUp(void) const
		{
			return c2.XYZ;
		}

		/* Gets the down vector of the matrix. */
		_Check_return_ inline Vector3 GetDown(void) const
		{
			return -GetUp();
		}

		/* Gets the forward vector of the matrix. */
		_Check_return_ inline Vector3 GetForward(void) const
		{
			return c3.XYZ;
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

		/* Calculates a quaternion that defines the orientation of this matrix. */
		_Check_return_ Quaternion GetOrientation(void) const;
		/* Gets the scale applied to this model. */
		_Check_return_ Vector3 GetScale(void) const;
		/* Calculates a matrix that defines the orientation and scale of this matrix. */
		_Check_return_ Matrix GetStatic(void) const;
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
		template<size_t _Dim>
		inline Vector4 _CrtGetColumn(const Matrix&);
		template<size_t _Dim>
		inline Vector4 _CrtSetColumn(Matrix&, Vector4);
		template<size_t _Dim>
		inline Vector4 _CrtGetRow(const Matrix&);
		template<size_t _Dim>
		inline Vector4 _CrtSetRow(Matrix&, Vector4);

		union
		{
			float f[16];

			/* This project is build to compile with the Microsoft compiler which allows this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
			struct
			{
				Vector4 c1, c2, c3, c4;
			};
#pragma warning(pop)
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

	/* Gets a specified column of a specified matrix. */
	template<size_t _Dim>
	_Check_return_ inline Vector4 _CrtGetColumn(_In_ const Matrix &m)
	{
		if constexpr (_Dim == 0) return m.c1;
		else if constexpr (_Dim == 1) return m.c2;
		else if constexpr (_Dim == 2) return m.c3;
		else if constexpr (_Dim == 3) return m.c4;
		else static_assert(true, "Matrix column index is out of range!");
	}

	/* Sets a specified column of a specified matrix. */
	template<size_t _Dim>
	_Check_return_ inline void _CrtSetColumn(_In_ Matrix &m, _In_ Vector4 v)
	{
		if constexpr (_Dim == 0) m.c1 = v;
		else if constexpr (_Dim == 1) m.c2 = v;
		else if constexpr (_Dim == 2) m.c3 = v;
		else if constexpr (_Dim == 3) m.c4 = v;
		else static_assert(true, "Matrix column index is out of range!");
	}

	/* Gets a specified row of a specified matrix. */
	template<size_t _Dim>
	_Check_return_ inline Vector4 _CrtGetRow(_In_ const Matrix &m)
	{
		if constexpr (_Dim == 0) return Vector4(m.c1.X, m.c2.X, m.c3.X, m.c4.X);
		else if constexpr (_Dim == 1) return Vector4(m.c1.Y, m.c2.Y, m.c3.Y, m.c4.Y);
		else if constexpr (_Dim == 2) return Vector4(m.c1.Z, m.c2.Z, m.c3.Z, m.c4.Z);
		else if constexpr (_Dim == 3) return Vector4(m.c1.W, m.c2.W, m.c3.W, m.c4.W);
		else static_assert(true, "Matrix row index is out of range!");
	}

	/* Sets a specified row of a specified matrix. */
	template<size_t _Dim>
	_Check_return_ inline void _CrtSetRow(_In_ Matrix &m, _In_ Vector4 v)
	{
		if constexpr (_Dim == 0)
		{
			m.c1.X = v.X;
			m.c2.X = v.Y;
			m.c3.X = v.Z;
			m.c4.X = v.W;
		}
		else if constexpr (_Dim == 1)
		{
			m.c1.Y = v.X;
			m.c2.Y = v.Y;
			m.c3.Y = v.Z;
			m.c4.Y = v.W;
		}
		else if constexpr (_Dim == 2)
		{
			m.c1.Z = v.X;
			m.c2.Z = v.Y;
			m.c3.Z = v.Z;
			m.c4.Z = v.W;
		}
		else if constexpr (_Dim == 3)
		{
			m.c1.W = v.X;
			m.c2.W = v.Y;
			m.c3.W = v.Z;
			m.c4.W = v.W;
		}
		else static_assert(true, "Matrix row index is out of range!");
	}
}