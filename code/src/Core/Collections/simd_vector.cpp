#include "Core/Collections/simd_vector.h"

const Pu::ofloat Pu::avxf_lut[8] =
{
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFF)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0, 0xFFFFFFFF, 0, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0, 0xFFFFFFFF, 0, 0, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0, 0xFFFFFFFF, 0, 0, 0, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0, 0xFFFFFFFF, 0, 0, 0, 0, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0)),
	_mm256_castsi256_ps(_mm256_set_epi32(0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0))
};

const Pu::int256 Pu::avxi_lut[8] =
{
	_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFF),
	_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0),
	_mm256_set_epi32(0, 0, 0, 0, 0, 0xFFFFFFFF, 0, 0),
	_mm256_set_epi32(0, 0, 0, 0, 0xFFFFFFFF, 0, 0, 0),
	_mm256_set_epi32(0, 0, 0, 0xFFFFFFFF, 0, 0, 0, 0),
	_mm256_set_epi32(0, 0, 0xFFFFFFFF, 0, 0, 0, 0, 0),
	_mm256_set_epi32(0, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0),
	_mm256_set_epi32(0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0)
};

const Pu::qfloat Pu::ssef_lut[4] =
{
	_mm_castsi128_ps(_mm_set_epi32(0, 0, 0, 0xFFFFFFFF)),
	_mm_castsi128_ps(_mm_set_epi32(0, 0, 0xFFFFFFFF, 0)),
	_mm_castsi128_ps(_mm_set_epi32(0, 0xFFFFFFFF, 0, 0)),
	_mm_castsi128_ps(_mm_set_epi32(0xFFFFFFFF, 0, 0, 0))
};