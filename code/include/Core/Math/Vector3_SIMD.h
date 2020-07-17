#pragma once
#include "Basics_SIMD.h"

namespace Pu
{
	/* Defines a union for 3D AVX vector streams. */
	union AVX_VEC3_UNION
	{
		/* This project is meant to be compiled with the Microsoft compiler which allows this extension. */
#pragma warning(push)
#pragma warning(disable:4201)
		struct
		{
			/* The single X components. */
			float X[8];
			/* The single Y components. */
			float Y[8];
			/* The single Z components. */
			float Z[8];
		};

		struct
		{
			/* The SIMD X component. */
			ofloat X8;
			/* The SIMD Y component. */
			ofloat Y8;
			/* The SIMD Z component. */
			ofloat Z8;
		};
#pragma warning(pop)
	};

	/* Initializes the specified AVX 3D vector union with the specified parameters at the specified index. */
	static inline void _mm256_seti_v3(_Inout_ AVX_VEC3_UNION &obj, _In_ float x, _In_ float y, _In_ float z, _In_ size_t i)
	{
		obj.X[i] = x;
		obj.Y[i] = y;
		obj.Z[i] = z;
	}

	/* Initializes the specified AVX streams from a 3D AVX vector union. */
	static inline void _mm256_set1_v3(_In_ const AVX_VEC3_UNION &obj, _Inout_ ofloat &x, _Inout_ ofloat &y, _Inout_ ofloat &z)
	{
		x = obj.X8;
		y = obj.Y8;
		z = obj.Z8;
	}

	/* Negates the specified 3D vector stream. */
	static inline void _mm256_neg_v3(_Inout_ ofloat &x, _Inout_ ofloat &y, _Inout_ ofloat &z, _In_ ofloat neg)
	{
		x = _mm256_mul_ps(x, neg);
		y = _mm256_mul_ps(y, neg);
		z = _mm256_mul_ps(z, neg);
	}

	/* Calculates the dot product of the two specified 3D vector streams. */
	_Check_return_ static inline ofloat _mm256_dot_v3(_In_ ofloat x1, _In_ ofloat y1, _In_ ofloat z1, _In_ ofloat x2, _In_ ofloat y2, _In_ ofloat z2)
	{
		return _mm256_add_ps(_mm256_mul_ps(x1, x2), _mm256_add_ps(_mm256_mul_ps(y1, y2), _mm256_mul_ps(z1, z2)));
	}

	/* Calculates the square magnitude of the specified 3D vector stream. */
	_Check_return_ static inline ofloat _mm256_len2_v3(_In_ ofloat x, _In_ ofloat y, _In_ ofloat z)
	{
		return _mm256_dot_v3(x, y, z, x, y, z);
	}

	/* Calculates the magnitude of the specified 3D vector stream. */
	_Check_return_ static inline ofloat _mm256_len_v3(_In_ ofloat x, _In_ ofloat y, _In_ ofloat z)
	{
		return _mm256_sqrt_ps(_mm256_len2_v3(x, y, z));
	}

	/* Normalizes the specified 3D vector stream (handles divide by zero). */
	static inline void _mm256_norm_v3(_Inout_ ofloat &x, _Inout_ ofloat &y, _Inout_ ofloat &z, _In_ ofloat zero)
	{
		const ofloat ll = _mm256_len2_v3(x, y, z);
		const ofloat l = _mm256_andnot_ps(_mm256_cmp_ps(zero, ll, _CMP_EQ_OQ), _mm256_rsqrt_ps(ll));

		x = _mm256_mul_ps(x, l);
		y = _mm256_mul_ps(y, l);
		z = _mm256_mul_ps(z, l);
	}

	/* Calculates the cross product between the two specified 3D vector streams. */
	static inline void _mm256_cross_v3(_In_ ofloat x1, _In_ ofloat y1, _In_ ofloat z1, _In_ ofloat x2, _In_ ofloat y2, _In_ ofloat z2, _Out_ ofloat &rx, _Out_ ofloat &ry, _Out_ ofloat &rz)
	{
		rx = _mm256_sub_ps(_mm256_mul_ps(y1, z2), _mm256_mul_ps(z1, y2));
		ry = _mm256_sub_ps(_mm256_mul_ps(z1, x2), _mm256_mul_ps(x1, z2));
		rz = _mm256_sub_ps(_mm256_mul_ps(x1, y2), _mm256_mul_ps(y1, x2));
	}

	/* Realloctes the specified areas of memory to match the new size. */
	static inline void _mm256_realloc_v3(_Inout_ ofloat *&x, _Inout_ ofloat *&y, _Inout_ ofloat *&z, _In_ size_t count)
	{
		x = _mm256_realloc_ps(x, count * 3);
		y = x + count;
		z = y + count;
	}

	/* Realloctes and clears the specified areas of memory to match the new size. */
	static inline void _mm256_recalloc_v3(_Inout_ ofloat *&x, _Inout_ ofloat *&y, _Inout_ ofloat *&z, _In_ size_t count)
	{
		x = _mm256_recalloc_ps(x, count * 3);
		y = x + count;
		z = y + count;
	}

	/* Deallocates the specified areas of memory. */
	static inline void _mm256_free_v3(_In_ ofloat *x, _In_ ofloat *y, _In_ ofloat *z)
	{
		_mm256_free_ps(x);
		(void)y;
		(void)z;
	}
}