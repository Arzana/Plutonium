#pragma once
#include "Core/String.h"
#include <random>

namespace Pu
{
	/* Defines an object that can generate noise with the square-diamond algorithm (midpoint displacement). */
	class SquareDiamondNoise
	{
	public:
		/* Initializes a new instance of of noise generator with the current time as its seed. */
		SquareDiamondNoise(void);
		/* Initializes a new instance of a noise generator with a specific integer seed. */
		SquareDiamondNoise(_In_ uint64 seed);
		/* Initializes a new instance of a noise generator with a specific string seed. */
		SquareDiamondNoise(_In_ const string &seed);
		SquareDiamondNoise(_In_ const SquareDiamondNoise&) = delete;
		/* Move constructor. */
		SquareDiamondNoise(_In_ SquareDiamondNoise &&value);
		/* Releases the resources allocated by the noise generator. */
		~SquareDiamondNoise(void);

		_Check_return_ SquareDiamondNoise& operator =(_In_ const SquareDiamondNoise&) = delete;
		/* Move assignment. */
		_Check_return_ SquareDiamondNoise& operator =(_In_ SquareDiamondNoise &&other);

		/* Sets the size of the heightmap (must be 2^x + 1). */
		void SetSize(_In_ uint16 size);
		/* Generates a new height map and returns it. */
		_Check_return_ const float* Generate(void);
		/* Generates a new height map within the range [0, 1]. */
		_Check_return_ const float* GenerateNormalized(void);

		/* Gets the last height map generated by the algorithm. */
		_Check_return_ inline const float* GetLastMap(void) const
		{
			return map;
		}

		/* Gets the width (and depth) of the heightmap. */
		_Check_return_ inline uint16 GetSize(void) const
		{
			return s;
		}

		/* Gets the last minimum height value generated by the algorithm. */
		_Check_return_ inline float GetMinimum(void) const
		{
			return minH;
		}

		/* Gets the last maximum height value generated by the algorithm. */
		_Check_return_ inline float GetMaximum(void) const
		{
			return maxH;
		}

		/* Sets the roughness of the randomness added to each vertex (0 means no randomness). */
		inline void SetRoughness(_In_ float value)
		{
			rgh = value;
		}

	private:
		std::default_random_engine rng;
		float *map;
		float minH, maxH, rgh;
		uint16 s;

		SquareDiamondNoise(std::default_random_engine rng);

		float RndFloat(void);
		float RndRangeFloat(uint16 range);
		void SetValue(int16 x, int16 z, float v);
		float GetValue(int16 x, int16 z) const;
		void GenerateInternal(uint16 size);
		void Square(int16 x, int16 z, uint16 reach);
		void Diamond(int16 x, int16 z, uint16 reach);
	};
}