#pragma once
#include "Basics_SIMD.h"

namespace Pu
{
	/* Defines a union for 4D AVX vector streams. */
	union AVX_VEC4_UNION
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
			/* The single W components. */
			float W[8];
		};

		struct
		{
			/* The SIMD X component. */
			ofloat X8;
			/* The SIMD Y component. */
			ofloat Y8;
			/* The SIMD Z component. */
			ofloat Z8;
			/* The SIMD W component. */
			ofloat W8;
		};
#pragma warning(pop)
	};

	/* Initializes the specified AVX 4D vector union with the specified parameters at the specified index. */
	static inline void _mm256_seti_v3(_Inout_ AVX_VEC4_UNION &obj, _In_ float x, _In_ float y, _In_ float z, _In_ float w, _In_ size_t i)
	{
		obj.X[i] = x;
		obj.Y[i] = y;
		obj.Z[i] = z;
		obj.W[i] = w;
	}

	/* Initializes the specified AVX streams from a 4D AVX vector union. */
	static inline void _mm256_set1_v4(_In_ const AVX_VEC4_UNION &obj, _Inout_ ofloat &x, _Inout_ ofloat &y, _Inout_ ofloat &z, _Inout_ ofloat &w)
	{
		x = obj.X8;
		y = obj.Y8;
		z = obj.Z8;
		w = obj.W8;
	}

	/* Calculates the dot product of the two specified 4D vector streams. */
	_Check_return_ static inline ofloat _mm256_dot_v4(_In_ ofloat x1, _In_ ofloat y1, _In_ ofloat z1, _In_ ofloat w1, _In_ ofloat x2, _In_ ofloat y2, _In_ ofloat z2, _In_ ofloat w2)
	{
		return _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(x1, x2), _mm256_mul_ps(y1, y2)), _mm256_add_ps(_mm256_mul_ps(z1, z2), _mm256_mul_ps(w1, w2)));
	}

	/* Calculates the square magnitude of the specified 4D vector stream. */
	_Check_return_ static inline ofloat _mm256_len2_v4(_In_ ofloat x, _In_ ofloat y, _In_ ofloat z, _In_ ofloat w)
	{
		return _mm256_dot_v4(x, y, z, w, x, y, z, w);
	}

	/* Calculates the magnitude of the specified 4D vector stream. */
	_Check_return_ static inline ofloat _mm256_len_v4(_In_ ofloat x, _In_ ofloat y, _In_ ofloat z, _In_ ofloat w)
	{
		return _mm256_sqrt_ps(_mm256_len2_v4(x, y, z, w));
	}

	/* Normalizes the specified 4D vector stream (handles divide by zero). */
	static inline void _mm256_norm_v4(_Inout_ ofloat &x, _Inout_ ofloat &y, _Inout_ ofloat &z, _Inout_ ofloat &w, _In_ ofloat zero)
	{
		const ofloat ll = _mm256_len2_v4(x, y, z, w);
		const ofloat l = _mm256_andnot_ps(_mm256_cmp_ps(zero, ll, _CMP_EQ_OQ), _mm256_rsqrt_ps(ll));

		x = _mm256_mul_ps(x, l);
		y = _mm256_mul_ps(y, l);
		z = _mm256_mul_ps(z, l);
		w = _mm256_mul_ps(w, l);
	}

	/* Realloctes the specified areas of memory to match the new size. */
	static inline void _mm256_realloc_v4(_Inout_ ofloat *&x, _Inout_ ofloat *&y, _Inout_ ofloat *&z, _Inout_ ofloat *&w, _In_ size_t count)
	{
		x = _mm256_realloc_ps(x, count << 2);
		y = x + count;
		z = y + count;
		w = z + count;
	}

	/* Realloctes and clears the specified areas of memory to match the new size. */
	static inline void _mm256_recalloc_v4(_Inout_ ofloat *&x, _Inout_ ofloat *&y, _Inout_ ofloat *&z, _Inout_ ofloat *&w, _In_ size_t count)
	{
		x = _mm256_recalloc_ps(x, count * 3);
		y = x + count;
		z = y + count;
		w = z + count;
	}

	/* Deallocates the specified areas of memory. */
	static inline void _mm256_free_v4(_In_ ofloat *x, _In_ ofloat *y, _In_ ofloat *z, _In_ ofloat *w)
	{
		_mm256_free_ps(x);
		(void)y;
		(void)z;
		(void)w;
	}
}