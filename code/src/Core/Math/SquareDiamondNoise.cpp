#include "Core/Math/SquareDiamondNoise.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Math/Interpolation.h"
#include <ctime>

Pu::SquareDiamondNoise::SquareDiamondNoise(void)
	: SquareDiamondNoise(std::default_random_engine(static_cast<uint32>(time(nullptr) & maxv<uint32>())))
{}

Pu::SquareDiamondNoise::SquareDiamondNoise(uint64 seed)
	: SquareDiamondNoise(std::default_random_engine(static_cast<uint32>(seed)))
{}

Pu::SquareDiamondNoise::SquareDiamondNoise(const string & seed)
	: SquareDiamondNoise(std::hash<string>{}(seed))
{}

Pu::SquareDiamondNoise::SquareDiamondNoise(SquareDiamondNoise && value)
	: rng(value.rng), map(value.map), s(value.s), minH(value.minH), maxH(value.maxH), rgh(value.rgh)
{
	map = nullptr;
}

Pu::SquareDiamondNoise::SquareDiamondNoise(std::default_random_engine rng)
	: rng(rng), s(65), minH(0.0f), maxH(0.0f), rgh(1.0f), map(nullptr)
{}

Pu::SquareDiamondNoise::~SquareDiamondNoise(void)
{
	if (map) free(map);
}

Pu::SquareDiamondNoise & Pu::SquareDiamondNoise::operator=(SquareDiamondNoise && other)
{
	if (this != &other)
	{
		if (map) free(map);

		rng = other.rng;
		map = other.map;
		s = other.s;
		minH = other.minH;
		maxH = other.maxH;
		rgh = other.rgh;

		other.map = nullptr;
	}

	return *this;
}

void Pu::SquareDiamondNoise::SetSize(uint16 size)
{
	/*
	Check for faulty inputs. 26 754
	Only valid inputs are:	5, 9, 17, 33, 65, 129, 257, 513, 1025, 2049, 4097, 8193 and 16385.
	After these we go above the uint32 limit of 26754^2 * 6
	*/
	if ((size - 1) & (size - 2) || (size - 1) & 0x3) Log::Fatal("Size is must be 2^x + 1!");

	if (map) free(map);
	s = size;
}

const float * Pu::SquareDiamondNoise::Generate(void)
{
	/*
	Allocate the buffer if needed.
	Default all values to zero except for the corners.
	*/
	if (!map)
	{
		map = reinterpret_cast<float*>(calloc(s * s, sizeof(float)));

		SetValue(0, 0, RndFloat());
		SetValue(s - 1, 0, RndFloat());
		SetValue(0, s - 1, RndFloat());
		SetValue(s - 1, s - 1, RndFloat());
	}

	minH = maxv<float>();
	maxH = minv<float>();

	/* Recursively generate the map. */
	GenerateInternal(s);
	return map;
}

const float * Pu::SquareDiamondNoise::GenerateNormalized(void)
{
	/* Generate the map like normal. */
	Generate();

	/* Scale the heightmap to uniform size. */
	const uint32 area = s * s;
	for (uint32 i = 0; i < area; i++)
	{
		map[i] = ilerp(minH, maxH, map[i]);
	}

	return map;
}

float Pu::SquareDiamondNoise::RndFloat(void)
{
	std::uniform_real_distribution<float> dist(0.0f, static_cast<float>(s));
	return dist(rng);
}

float Pu::SquareDiamondNoise::RndRangeFloat(uint16 range)
{
	const float frange = static_cast<float>(range);
	std::uniform_real_distribution<float> dist(-frange, frange);
	return rgh * dist(rng);
}

void Pu::SquareDiamondNoise::SetValue(int16 x, int16 z, float v)
{
	map[z * s + x] = v;
	minH = min(minH, v);
	maxH = max(maxH, v);
}

float Pu::SquareDiamondNoise::GetValue(int16 x, int16 z) const
{
	return map[z * s + x];
}

void Pu::SquareDiamondNoise::GenerateInternal(uint16 size)
{
	/* Check if we've reached the furthest depth. */
	const uint16 hs = size / 2;
	if (hs < 1) return;

	/* Perform square steps. */
	for (uint16 z = hs; z < s; z += size)
	{
		for (uint16 x = hs; x < s; x += size)
		{
			Square(x % s, z % s, hs);
		}
	}

	/* Perform diamond steps. */
	for (uint16 x = 0, c = 1; x < s; x += hs, c++)
	{
		/* Check if column is odd. */
		if (c & 1)
		{
			for (uint16 z = hs; z < s; z += size)
			{
				Diamond(x % s, z % s, hs);
			}
		}
		else
		{
			for (uint16 z = 0; z < s; z += size)
			{
				Diamond(x % s, z % s, hs);
			}
		}
	}

	/* Repeat untill we've reached max depth. */
	GenerateInternal(hs);
}

void Pu::SquareDiamondNoise::Square(int16 x, int16 z, uint16 reach)
{
	uint32 count = 0;
	float sum = 0.0f;

	if (x - reach >= 0)
	{
		if (z - reach >= 0)
		{
			sum += GetValue(x - reach, z - reach);
			++count;
		}

		if (z + reach < s)
		{
			sum += GetValue(x - reach, z + reach);
			++count;
		}
	}

	if (x + reach < s)
	{
		if (z - reach >= 0)
		{
			sum += GetValue(x + reach, z - reach);
			++count;
		}

		if (z + reach < s)
		{
			sum += GetValue(x + reach, z + reach);
			++count;
		}
	}

	SetValue(x, z, (sum + RndRangeFloat(reach)) / count);
}

void Pu::SquareDiamondNoise::Diamond(int16 x, int16 z, uint16 reach)
{
	int32 count = 0;
	float sum = 0.0f;

	if (x - reach >= 0)
	{
		sum += GetValue(x - reach, z);
		++count;
	}

	if (x + reach < s)
	{
		sum += GetValue(x + reach, z);
		++count;
	}

	if (z - reach >= 0)
	{
		sum += GetValue(x, z - reach);
		++count;
	}

	if (z + reach < s)
	{
		sum += GetValue(x, z + reach);
		++count;
	}

	SetValue(x, z, (sum + RndRangeFloat(reach)) / count);
}