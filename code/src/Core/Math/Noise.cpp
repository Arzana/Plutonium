#include "Core/Math/Noise.h"
#include "Core/Math/Interpolation.h"
#include <ctime>

Pu::Noise::Noise(void)
	: Noise(std::default_random_engine(static_cast<uint32>(time(nullptr))))
{}

Pu::Noise::Noise(uint64 seed)
	: Noise(std::default_random_engine(static_cast<uint32>(seed)))
{}

Pu::Noise::Noise(const string & seed)
	: Noise(std::hash<string>{}(seed))
{}

Pu::Noise::Noise(const Noise & value)
{
	permutations = reinterpret_cast<byte*>(malloc(512));
	memcpy(permutations, value.permutations, 512);
}

Pu::Noise::Noise(Noise && value)
	: permutations(value.permutations)
{
	value.permutations = nullptr;
}

Pu::Noise::~Noise(void)
{
	if (permutations) free(permutations);
}

Pu::Noise & Pu::Noise::operator=(const Noise & other)
{
	if (this != &other) memcpy(permutations, other.permutations, 512);
	return *this;
}

Pu::Noise & Pu::Noise::operator=(Noise && other)
{
	if (this != &other)
	{
		if (permutations) free(permutations);
		permutations = other.permutations;

		other.permutations = nullptr;
	}

	return *this;
}

Pu::Noise::Noise(std::default_random_engine engine)
{
	permutations = reinterpret_cast<byte*>(malloc(512));

	/* Initialize the list to have values from 0 to 256. */
	for (uint16 i = 0; i < 256; i++) permutations[i] = static_cast<byte>(i);

	/* Shuffle the list and copy it once afterwards to avoid invalid memory access. */
	std::shuffle(permutations, permutations + 256, engine);
	memcpy(permutations + 256, permutations, 256);
}

float Pu::Noise::Octave(float x) const
{
	/* Find the unit that contains the point. */
	const float fx = floorf(x);
	const size_t ifx = static_cast<size_t>(fx) & 0xFF;

	/* Hash the coodinates of the line. */
	const size_t a = permutations[ifx];
	const size_t b = permutations[ifx + 1];

	/*
	Find the relative x of the point in the cube.
	x will become that fractional part (fpart) of its origional value, we're nor using fpart to speed up the calculation.
	*/
	const float u = Fade(x -= fx);

	/* Add blended results from the end segments. */
	const float n0 = Gradient(a, x);
	const float n1 = Gradient(b, x - 1.0f);
	return lerp(n0, n1, u);
}

float Pu::Noise::Octave(float x, float y) const
{
	const float fx = floorf(x);
	const float fy = floorf(y);

	const size_t ifx = static_cast<size_t>(fx) & 0xFF;
	const size_t ify = static_cast<size_t>(fy) & 0xFF;

	const size_t a = permutations[permutations[ifx] + ify];
	const size_t b = permutations[permutations[ifx + 1] + ify];
	const size_t ab = permutations[permutations[ifx] + ify + 1];
	const size_t ba = permutations[permutations[ifx + 1] + ify + 1];

	const float u = Fade(x -= fx);
	const float v = Fade(y -= fy);

	float n0 = Gradient(a, x, y);
	float n1 = Gradient(b, x - 1.0f, y);
	float u0 = lerp(n0, n1, u);
	n0 = Gradient(ab, x, y - 1.0f);
	n1 = Gradient(ba, x - 1.0f, y - 1.0f);
	return lerp(u0, lerp(n0, n1, u), v);
}

float Pu::Noise::Octave(float x, float y, float z) const
{
	const float fx = floorf(x);
	const float fy = floorf(y);
	const float fz = floorf(z);

	const size_t ifx = static_cast<size_t>(fx) & 0xFF;
	const size_t ify = static_cast<size_t>(fy) & 0xFF;
	const size_t ifz = static_cast<size_t>(fz) & 0xFF;

	const size_t a = permutations[ifx] + ify;
	const size_t b = permutations[ifx + 1] + ify;
	const size_t aa = permutations[a] + ifz;
	const size_t ab = permutations[a + 1] + ify;
	const size_t ba = permutations[b] + ifz;
	const size_t bb = permutations[b + 1] + ifz;

	const float u = Fade(x -= fx);
	const float v = Fade(y -= fy);
	const float w = Fade(z -= fz);

	float n0 = Gradient(aa, x, y, z);
	float n1 = Gradient(ba, x - 1.0f, y, z);
	float u0 = lerp(n0, n1, u);
	n0 = Gradient(ab, x, y - 1.0f, z);
	n1 = Gradient(bb, x - 1.0f, y - 1.0f, z);
	float u1 = lerp(n0, n1, u);
	float v0 = lerp(u0, u1, v);
	n0 = Gradient(aa + 1, x, y, z - 1.0f);
	n1 = Gradient(ba + 1, x - 1.0f, y, z - 1.0f);
	u0 = lerp(n0, n1, u);
	n0 = Gradient(ab + 1, x, y - 1.0f, z - 1.0f);
	n1 = Gradient(bb + 1, x - 1.0f, y - 1.0f, z - 1.0f);
	u1 = lerp(n0, n1, u);
	return lerp(v0, lerp(u0, u1, v), w);
}

float Pu::Noise::NormalizedOctave(float x) const
{
	return (Octave(x) + 1.0f) * 0.5f;
}

float Pu::Noise::NormalizedOctave(float x, float y) const
{
	return (Octave(x, y) + 1.0f) * 0.5f;
}

float Pu::Noise::NormalizedOctave(float x, float y, float z) const
{
	return (Octave(x, y, z) + 1.0f) * 0.5f;
}

float Pu::Noise::Scale(float x, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += Octave(x * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::Scale(float x, float y, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += Octave(x * frequency, y * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::Scale(float x, float y, float z, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += Octave(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::NormalizedScale(float x, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += NormalizedOctave(x * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::NormalizedScale(float x, float y, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += NormalizedOctave(x * frequency, y * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::NormalizedScale(float x, float y, float z, size_t octaves, float persistance, float lacunatity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunatity)
	{
		result += NormalizedOctave(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}

float Pu::Noise::Fade(float t)
{
	/* Simple cubic fade function. */
	return cube(t) * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Pu::Noise::Gradient(size_t i, float x) const
{
	/* Gets the ends of the unit line represented by x. */
	return (permutations[i] & 1) ? -x : x;
}

float Pu::Noise::Gradient(size_t i, float x, float y) const
{
	/* Get the corners of the unit rectangle represented by [x, y] */
	const byte h = permutations[i];
	return ((h & 1) ? -x : x) + ((h & 2) ? -y : y);
}

float Pu::Noise::Gradient(size_t i, float x, float y, float z) const
{
	/*
	This basically gets the corners of the unit cube represented by [x, y, z]. 
	It's optimized to use minimal branching.
	*/
	const byte h = permutations[i] & 0xF;
	
	const float u = h < 8 ? x : y;
	const float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}