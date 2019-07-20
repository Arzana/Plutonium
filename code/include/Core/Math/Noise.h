#pragma once
#include <random>
#include "Vector3.h"

namespace Pu
{
	/* Defines a pseudo random Perlin noise generator. */
	class Noise
	{
	public:
		/* Initializes a new instance of a noise generator with the current time as its seed. */
		Noise(void);
		/* Initializes a new instance of a noise generator with a specific integer seed. */
		Noise(_In_ uint64 seed);
		/* Initializes a new instance of a noise generator with a specific string seed. */
		Noise(_In_ const string &seed);
		/* Copy constructor. */
		Noise(_In_ const Noise &value);
		/* Move constructor. */
		Noise(_In_ Noise &&value);
		/* Releases the resources allocated by the noise generator. */
		~Noise(void);

		/* Copy assignment. */
		_Check_return_ Noise& operator =(_In_ const Noise &other);
		/* Move assignment. */
		_Check_return_ Noise& operator =(_In_ Noise &&other);

		/* Gets a noise value for the specified point. */
		_Check_return_ float Octave(_In_ float x) const;
		/* Gets a noise value for the specified 2D coordinate. */
		_Check_return_ float Octave(_In_ float x, _In_ float y) const;
		/* Gets a noise value for the specified 3D coordinate. */
		_Check_return_ float Octave(_In_ float x, _In_ float y, _In_ float z) const;
		/* Gets a noise value for multiple octaves of noise for the specified point. */
		_Check_return_ float Scale(_In_ float x, _In_ size_t octaves, _In_ float persistance, _In_ float lacunatity) const;
		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ float Scale(_In_ float x, _In_ float y, _In_ size_t octaves, _In_ float persistance, _In_ float lacunatity) const;
		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ float Scale(_In_ float x, _In_ float y, _In_ float z, _In_ size_t octaves, _In_ float persistance, _In_ float lacunatity) const;

		/* Gets a noise value for the specified 2D coordinate. */
		_Check_return_ inline float Octave(_In_ Vector2 p) const
		{
			return Octave(p.X, p.Y);
		}

		/* Gets a noise value for the specified 3D coordinate. */
		_Check_return_ inline float Octave(_In_ Vector3 p) const
		{
			return Octave(p.X, p.Y, p.Z);
		}

		/* Gets a noise value for multiple octaves of noise for the 2D coordinate. */
		_Check_return_ inline float Scale(_In_ Vector2 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunatity) const
		{
			return Scale(p.X, p.Y, octaves, persistance, lacunatity);
		}

		/* Gets a noise value for multiple octaves of noise for the 3D coordinate. */
		_Check_return_ inline float Scale(_In_ Vector3 p, _In_ size_t octaves, _In_ float persistance, _In_ float lacunatity) const
		{
			return Scale(p.X, p.Y, p.Z, octaves, persistance, lacunatity);
		}

	private:
		byte *permutations;

		Noise(std::default_random_engine engine);

		static float Fade(float t);

		float Gradient(size_t i, float x) const;
		float Gradient(size_t i, float x, float y) const;
		float Gradient(size_t i, float x, float y, float z) const;
	};
}