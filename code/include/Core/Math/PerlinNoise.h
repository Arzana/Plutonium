#pragma once
#include <random>
#include "Vector3.h"

namespace Pu
{
	/* 
	Defines a pseudo random Perlin noise generator.
	An octave defines a single slice of the Perlin noise algorithm, this is in the range [-1, 1].
	Normalized options move this range from [-1, 1] to a [0, 1] range for ease of use.

	You can also use multiple octaves in one function call, the frequency and amplitude are controlled by lacunarity and persistance respectively.
	Lacunarity controlls the frequency as follows: lacunarity^octave, the range of the lacunarity should be [0, -]
	Persistance controlls the amplitude as follows: persistance^octave, the range of persistance should be [0, 1]
	*/
	class PerlinNoise
	{
	public:
		/* Initializes a new instance of a noise generator with the current time as its seed. */
		PerlinNoise(void);
		/* Initializes a new instance of a noise generator with a specific integer seed. */
		PerlinNoise(_In_ uint64 seed);
		/* Initializes a new instance of a noise generator with a specific string seed. */
		PerlinNoise(_In_ const string &seed);
		/* Copy constructor. */
		PerlinNoise(_In_ const PerlinNoise &value);
		/* Move constructor. */
		PerlinNoise(_In_ PerlinNoise &&value);
		/* Releases the resources allocated by the noise generator. */
		~PerlinNoise(void);

		/* Copy assignment. */
		_Check_return_ PerlinNoise& operator =(_In_ const PerlinNoise &other);
		/* Move assignment. */
		_Check_return_ PerlinNoise& operator =(_In_ PerlinNoise &&other);

		/* Gets a noise value (range [-1, 1]) for the specified point. */
		_Check_return_ float Octave(_In_ float x) const;
		/* Gets a noise value (range [-1, 1]) for the specified 2D coordinate. */
		_Check_return_ float Octave(_In_ float x, _In_ float y) const;
		/* Gets a noise value (range [-1, 1]) for the specified 3D coordinate. */
		_Check_return_ float Octave(_In_ float x, _In_ float y, _In_ float z) const;
		/* Gets a noise value (range [0, 1]) for the specified point. */
		_Check_return_ float NormalizedOctave(_In_ float x) const;
		/* Gets a noise value (range [0, 1]) for the specified 2D coordinate. */
		_Check_return_ float NormalizedOctave(_In_ float x, _In_ float y) const;
		/* Gets a noise value (range [0, 1]) for the specified 3D coordinate. */
		_Check_return_ float NormalizedOctave(_In_ float x, _In_ float y, _In_ float z) const;

		/* Gets a noise value for multiple octaves of noise for the specified point. */
		_Check_return_ float Scale(_In_ float x, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;
		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ float Scale(_In_ float x, _In_ float y, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;
		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ float Scale(_In_ float x, _In_ float y, _In_ float z, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;
		/* Gets a noise value for multiple octaves of noise for the specified point. */
		_Check_return_ float NormalizedScale(_In_ float x, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;
		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ float NormalizedScale(_In_ float x, _In_ float y, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;
		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ float NormalizedScale(_In_ float x, _In_ float y, _In_ float z, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const;

		/* Gets a noise value (range [-1, 1]) for the specified 2D coordinate. */
		_Check_return_ inline float Octave(_In_ Vector2 p) const
		{
			return Octave(p.X, p.Y);
		}

		/* Gets a noise value (range [-1, 1]) for the specified 3D coordinate. */
		_Check_return_ inline float Octave(_In_ Vector3 p) const
		{
			return Octave(p.X, p.Y, p.Z);
		}

		/* Gets a noise value (range [0, 1]) for the specified 2D coordinate. */
		_Check_return_ inline float NormalizedOctave(_In_ Vector2 p) const
		{
			return NormalizedOctave(p.X, p.Y);
		}

		/* Gets a noise value (range [0, 1]) for the specified 3D coordinate. */
		_Check_return_ inline float NormalizedOctave(_In_ Vector3 p) const
		{
			return NormalizedOctave(p.X, p.Y, p.Z);
		}

		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ inline float Scale(_In_ Vector2 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const
		{
			return Scale(p.X, p.Y, octaves, persistance, lacunarity);
		}

		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ inline float Scale(_In_ Vector3 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const
		{
			return Scale(p.X, p.Y, p.Z, octaves, persistance, lacunarity);
		}

		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ inline float NormalizedScale(_In_ Vector2 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const
		{
			return NormalizedScale(p.X, p.Y, octaves, persistance, lacunarity);
		}

		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ inline float NormalizedScale(_In_ Vector3 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunarity) const
		{
			return NormalizedScale(p.X, p.Y, p.Z, octaves, persistance, lacunarity);
		}

	private:
		byte *permutations;

		PerlinNoise(std::default_random_engine engine);

		static float Fade(float t);

		float Gradient(size_t i, float x) const;
		float Gradient(size_t i, float x, float y) const;
		float Gradient(size_t i, float x, float y, float z) const;
	};
}