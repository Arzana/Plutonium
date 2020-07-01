#include "Core/Collections/fvector.h"

__declspec(align(32)) const Pu::ofloat Pu::fvector::lut[8] =
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

void Pu::fvector::erase(size_t idx)
{
	/* We can just decrease the count and early out if this was the last element. */
	assert(idx < cnt && "Index out of range!");
	if (idx == --cnt) return;

	const size_t i = idx >> 0x3;
	const size_t j = idx & 0x7;
	AVX_FLOAT_UNION shift;

	/* 
	This wasn't the last element so we must swift all following element left by one. 
	Start at the back and work our way back to the removed element.
	*/
	float carryIn = -NAN, carryOut;
	for (size_t k = cnt >> 0x3; k > i; k--)
	{
		shift.AVX = buffer[k];
		carryOut = shift.V[0];

		shift.V[0] = shift.V[1];
		shift.V[1] = shift.V[2];
		shift.V[2] = shift.V[3];
		shift.V[3] = shift.V[4];
		shift.V[4] = shift.V[5];
		shift.V[5] = shift.V[6];
		shift.V[6] = shift.V[7];
		shift.V[7] = carryIn;

		buffer[k] = shift.AVX;
		carryIn = carryOut;
	}

	/* 
	Handle the last AVX type differently.
	This is because it doesn't have a carry out.
	*/
	shift.AVX = buffer[i];
	for (size_t k = j; k < 7; k++)
	{
		shift.V[k] = shift.V[k + 1];
	}

	/* But it does have a carry in. */
	shift.V[7] = carryIn;
	buffer[i] = shift.AVX;
}