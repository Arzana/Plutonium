#include "Core/Math/PerlinNoise.h"
#include "Core/Math/Interpolation.h"
#include <ctime>

Pu::PerlinNoise::PerlinNoise(void)
	: PerlinNoise(std::default_random_engine(static_cast<uint32>(time(nullptr) & maxv<uint32>())))
{}

Pu::PerlinNoise::PerlinNoise(uint64 seed)
	: PerlinNoise(std::default_random_engine(static_cast<uint32>(seed)))
{}

Pu::PerlinNoise::PerlinNoise(const string & seed)
	: PerlinNoise(std::hash<string>{}(seed))
{}

Pu::PerlinNoise::PerlinNoise(const PerlinNoise & value)
{
	permutations = reinterpret_cast<byte*>(malloc(512));
	memcpy(permutations, value.permutations, 512);
}

Pu::PerlinNoise::PerlinNoise(PerlinNoise && value)
	: permutations(value.permutations)
{
	value.permutations = nullptr;
}

Pu::PerlinNoise::PerlinNoise(std::default_random_engine engine)
{
	permutations = reinterpret_cast<byte*>(malloc(512));

	/* Initialize the list to have values from 0 to 256. */
	for (uint16 i = 0; i < 256; i++) permutations[i] = static_cast<byte>(i);

	/* Shuffle the list and copy it once afterwards to avoid invalid memory access. */
	std::shuffle(permutations, permutations + 256, engine);
	memcpy(permutations + 256, permutations, 256);
}

Pu::PerlinNoise::~PerlinNoise(void)
{
	if (permutations) free(permutations);
}

Pu::PerlinNoise & Pu::PerlinNoise::operator=(const PerlinNoise & other)
{
	if (this != &other) memcpy(permutations, other.permutations, 512);
	return *this;
}

Pu::PerlinNoise & Pu::PerlinNoise::operator=(PerlinNoise && other)
{
	if (this != &other)
	{
		if (permutations) free(permutations);
		permutations = other.permutations;

		other.permutations = nullptr;
	}

	return *this;
}

float Pu::PerlinNoise::Octave(float x) const
{
	/* Find the unit that contains the point. */
	const float fx = floorf(x);
	const uint32 ifx = static_cast<uint32>(fx) & 0xFF;

	/* Hash the coodinates of the line. */
	const uint32 a = permutations[ifx];
	const uint32 b = permutations[ifx + 1];

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

float Pu::PerlinNoise::Octave(float x, float y) const
{
	const float fx = floorf(x);
	const float fy = floorf(y);

	const uint32 ifx = static_cast<uint32>(fx) & 0xFF;
	const uint32 ify = static_cast<uint32>(fy) & 0xFF;

	const uint32 a = permutations[permutations[ifx] + ify];
	const uint32 b = permutations[permutations[ifx + 1] + ify];
	const uint32 ab = permutations[permutations[ifx] + ify + 1];
	const uint32 ba = permutations[permutations[ifx + 1] + ify + 1];

	const float u = Fade(x -= fx);
	const float v = Fade(y -= fy);

	float n0 = Gradient(a, x, y);
	float n1 = Gradient(b, x - 1.0f, y);
	float u0 = lerp(n0, n1, u);
	n0 = Gradient(ab, x, y - 1.0f);
	n1 = Gradient(ba, x - 1.0f, y - 1.0f);
	return lerp(u0, lerp(n0, n1, u), v);
}

float Pu::PerlinNoise::Octave(float x, float y, float z) const
{
	const float fx = floorf(x);
	const float fy = floorf(y);
	const float fz = floorf(z);

	const uint32 ifx = static_cast<uint32>(fx) & 0xFF;
	const uint32 ify = static_cast<uint32>(fy) & 0xFF;
	const uint32 ifz = static_cast<uint32>(fz) & 0xFF;

	const uint32 a = permutations[ifx] + ify;
	const uint32 b = permutations[ifx + 1] + ify;
	const uint32 aa = permutations[a] + ifz;
	const uint32 ab = permutations[a + 1] + ifz;
	const uint32 ba = permutations[b] + ifz;
	const uint32 bb = permutations[b + 1] + ifz;

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

float Pu::PerlinNoise::NormalizedOctave(float x) const
{
	return (Octave(x) + 1.0f) * 0.5f;
}

float Pu::PerlinNoise::NormalizedOctave(float x, float y) const
{
	return (Octave(x, y) + 1.0f) * 0.5f;
}

float Pu::PerlinNoise::NormalizedOctave(float x, float y, float z) const
{
	return (Octave(x, y, z) + 1.0f) * 0.5f;
}

float Pu::PerlinNoise::Scale(float x, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += Octave(x * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::Scale(float x, float y, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += Octave(x * frequency, y * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::Scale(float x, float y, float z, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += Octave(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::NormalizedScale(float x, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += NormalizedOctave(x * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::NormalizedScale(float x, float y, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += NormalizedOctave(x * frequency, y * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::NormalizedScale(float x, float y, float z, size_t octaves, float persistance, float lacunarity) const
{
	float amplitude = 1.0f;
	float frequency = 1.0f;
	float result = 0.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance, frequency *= lacunarity)
	{
		result += NormalizedOctave(x * frequency, y * frequency, z * frequency) * amplitude;
	}

	return result;
}

float Pu::PerlinNoise::Max(size_t octaves, float persistance)
{
	float result = 0.0f;
	float amplitude = 1.0f;

	for (size_t i = 0; i < octaves; i++, amplitude *= persistance)
	{
		result += amplitude;
	}

	return result;
}

float Pu::PerlinNoise::Fade(float t)
{
	/* Simple cubic fade function. */
	return cube(t) * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float Pu::PerlinNoise::Gradient(uint32 i, float x) const
{
	/* Gets the ends of the unit line represented by x. */
	return (permutations[i] & 1) ? -x : x;
}

float Pu::PerlinNoise::Gradient(uint32 i, float x, float y) const
{
	/* Get the corners of the unit rectangle represented by [x, y] */
	const byte h = permutations[i];
	return ((h & 1) ? -x : x) + ((h & 2) ? -y : y);
}

float Pu::PerlinNoise::Gradient(uint32 i, float x, float y, float z) const
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