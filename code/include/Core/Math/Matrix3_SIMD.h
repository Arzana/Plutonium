#pragma once
#include "Basics_SIMD.h"

namespace Pu
{
	/* Defines a union for 3x3 AVX matrix streams. */
	union AVX_MAT3_UNION
	{
		/* This project is meant to be compiled with the Microsoft compiler which allows this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
		struct
		{
			float M00[8];
			float M01[8];
			float M02[8];
			float M10[8];
			float M11[8];
			float M12[8];
			float M20[8];
			float M21[8];
			float M22[8];
		};

		struct
		{
			ofloat M008;
			ofloat M018;
			ofloat M028;
			ofloat M108;
			ofloat M118;
			ofloat M128;
			ofloat M208;
			ofloat M218;
			ofloat M228;
		};
#pragma warning(pop)
	};

	/* Initializes the specified AVX 3x3 matrix union to an identity matrix at the specified index. */
	static inline void _mm256_setzero_m3(_Inout_ AVX_MAT3_UNION &obj, _In_ size_t i)
	{
		obj.M00[i] = 1.0f;
		obj.M01[i] = 0.0f;
		obj.M02[i] = 0.0f;
		obj.M10[i] = 0.0f;
		obj.M11[i] = 1.0f;
		obj.M12[i] = 0.0f;
		obj.M20[i] = 0.0f;
		obj.M21[i] = 0.0f;
		obj.M22[i] = 1.0f;
	}

	/* Initializes the specified AVX 3x3 matrix union with the specified parameters at the specified index. */
	static inline void _mm256_seti_m3(_Inout_ AVX_MAT3_UNION &obj, _In_ const float components[9], _In_ size_t i)
	{
		obj.M00[i] = components[0];
		obj.M01[i] = components[1];
		obj.M02[i] = components[2];
		obj.M10[i] = components[3];
		obj.M11[i] = components[4];
		obj.M12[i] = components[5];
		obj.M20[i] = components[6];
		obj.M21[i] = components[7];
		obj.M22[i] = components[8];
	}

	/* Initializes the specified AVX streams from a 3x3 AVX matrix union. */
	static inline void _mm256_set1_m3(_In_ const AVX_MAT3_UNION &obj, _Inout_ ofloat &m00, _Inout_ ofloat &m01, _Inout_ ofloat &m02, _Inout_ ofloat &m10, _Inout_ ofloat &m11, _Inout_ ofloat &m12, _Inout_ ofloat &m20, _Inout_ ofloat &m21, _Inout_ ofloat &m22)
	{
		m00 = obj.M008;
		m01 = obj.M018;
		m02 = obj.M028;
		m10 = obj.M108;
		m11 = obj.M118;
		m12 = obj.M128;
		m20 = obj.M208;
		m21 = obj.M218;
		m22 = obj.M228;
	}

	/* Multiplies the specified 3x3 Matrix stream by the specified 3D vector stream. */
	static inline void _mm256_mat3mul_v3(_In_ ofloat m00, _In_ ofloat m01, _In_ ofloat m02, _In_ ofloat m10, _In_ ofloat m11, _In_ ofloat m12, _In_ ofloat m20, _In_ ofloat m21, _In_ ofloat m22, _In_ ofloat x, _In_ ofloat y, _In_ ofloat z, _Out_ ofloat &rx, _Out_ ofloat &ry, _Out_ ofloat &rz)
	{
		rx = _mm256_add_ps(_mm256_mul_ps(x, m00), _mm256_add_ps(_mm256_mul_ps(y, m10), _mm256_mul_ps(z, m20)));
		ry = _mm256_add_ps(_mm256_mul_ps(x, m01), _mm256_add_ps(_mm256_mul_ps(y, m11), _mm256_mul_ps(z, m21)));
		rz = _mm256_add_ps(_mm256_mul_ps(x, m02), _mm256_add_ps(_mm256_mul_ps(y, m12), _mm256_mul_ps(z, m22)));
	}

	/* Realloctes the specified areas of memory to match the new size. */
	static inline void _mm256_realloc_m3(_Inout_ ofloat *&m00, _Inout_ ofloat *&m01, _Inout_ ofloat *&m02, _Inout_ ofloat *&m10, _Inout_ ofloat *&m11, _Inout_ ofloat *&m12, _Inout_ ofloat *&m20, _Inout_ ofloat *&m21, _Inout_ ofloat *&m22, _In_ size_t count)
	{
		m00 = _mm256_realloc_ps(m00, count * 9);
		m01 = m00 + count;
		m02 = m01 + count;
		m10 = m02 + count;
		m11 = m10 + count;
		m12 = m11 + count;
		m20 = m12 + count;
		m21 = m20 + count;
		m22 = m21 + count;
	}

	/* Deallocates the specified areas of memory. */
	static inline void _mm256_free_m3(_In_ ofloat *m00, _In_ ofloat *m01, _In_ ofloat *m02, _In_ ofloat *m10, _In_ ofloat *m11, _In_ ofloat *m12, _In_ ofloat *m20, _In_ ofloat *m21, _In_ ofloat *m22)
	{
		_mm256_free_ps(m00);
		(void)m01;
		(void)m02;
		(void)m10;
		(void)m11;
		(void)m12;
		(void)m20;
		(void)m21;
		(void)m22;
	}
}