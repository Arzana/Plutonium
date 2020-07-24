#pragma once
#include "Constants.h"

namespace Pu
{
	/* Raises the input value to the power of two. */
	_Check_return_ inline ofloat _mm256_sqr_ps(_In_ ofloat v)
	{
		return _mm256_mul_ps(v, v);
	}

	/* Raises the input value to the power of three. */
	_Check_return_ inline ofloat _mm256_cube_ps(_In_ ofloat v)
	{
		return _mm256_mul_ps(_mm256_mul_ps(v, v), v);
	}

	/* Performs a safe division. */
	_Check_return_ inline ofloat _mm256_divs_ps(_In_ ofloat num, _In_ ofloat denom, _In_ ofloat zero)
	{
		return _mm256_andnot_ps(_mm256_cmp_ps(zero, denom, _CMP_EQ_OQ), _mm256_div_ps(num, denom));
	}

	/* Clamps the specified packed single between the specified minimum and maximum. */
	_Check_return_ inline ofloat _mm256_clamp_ps(_In_ ofloat v, _In_ ofloat a, _In_ ofloat b)
	{
		return _mm256_max_ps(a, _mm256_min_ps(b, v));
	}

	/* Realoctes the specified area of memory. */
	_Check_return_ static inline ofloat* _mm256_realloc_ps(_In_ ofloat *block, _In_ size_t count)
	{
		return reinterpret_cast<ofloat*>(_aligned_realloc(block, sizeof(ofloat) * count, sizeof(ofloat)));
	}

	/* Reallocates and clears the specified area of memory. */
	_Check_return_ static inline ofloat* _mm256_recalloc_ps(_In_ ofloat *block, _In_ size_t count)
	{
		return reinterpret_cast<ofloat*>(_aligned_recalloc(block, count, sizeof(ofloat) * count, sizeof(ofloat)));
	}

	/* Deallocates the specifie area of memory. */
	static inline void _mm256_free_ps(_In_ ofloat *block)
	{
		_aligned_free(block);
	}
}