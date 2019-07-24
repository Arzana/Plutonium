#pragma once
#include "Graphics/Models/Mesh.h"
#include "Core/Math/PerlinNoise.h"

namespace Pu
{
	struct Terrain;

	/* Defines a helper object for generating terrains easily (uses unnormalized perlin noise). */
	class TerrainCreator
	{
	public:
		/* Initializes a new instance of a terrain generator with the specified noise creator. */
		TerrainCreator(_In_ const PerlinNoise &noise);
		/* Copy constructor. */
		TerrainCreator(_In_ const TerrainCreator&) = default;
		/* Move constructor. */
		TerrainCreator(_In_ TerrainCreator&&) = default;

		/* Copy assignment. */
		_Check_return_ TerrainCreator& operator =(_In_ const TerrainCreator&) = default;
		/* Move assignment. */
		_Check_return_ TerrainCreator& operator =(_In_ TerrainCreator&&) = default;

		/* Gets the required size of the next buffer to be generated with this generator. */
		_Check_return_ size_t GetBufferSize(void) const;
		/* Populates the source buffer with the generated terrain at the specified 2D offset and returns a mesh that can view that terrain in the destination buffer. */
		_Check_return_ Mesh Generate(_In_ Buffer &src, _In_ const Buffer &dst, _In_ Vector2 position);

		/* Sets just the width of the terrain. */
		inline void SetWidth(_In_ byte value)
		{
			w = value;
		}

		/* Sets just the depth of the terrain. */
		inline void SetDepth(_In_ byte value)
		{
			d = value;
		}

		/* Sets the width and depth of the terrain. */
		inline void SetDimensions(_In_ byte width, _In_ byte depth)
		{
			w = width;
			d = depth;
		}

		/* Sets the number of octaves to use whilst generating terrain. */
		inline void SetOctaveCount(_In_ size_t value)
		{
			o = value;
		}

		/* Sets the scale that should be applied to coordinates during generation (doesn't affect mesh scale). */
		inline void SetCoordinateScale(_In_ float value)
		{
			s = value;
		}

		/* 
		Sets the decrease in amplitude of octaves (should be in range [0, 1]). 
		Increasing this will increase in influence of small features. 
		*/
		inline void SetPersistance(_In_ float value)
		{
			p = value;
		}

		/* 
		Sets the increase in frequency of octaves (should be > 1).
		Increase this for more small features.
		*/
		inline void SetLacunarity(_In_ float value)
		{
			l = value;
		}

		/* Sets all the scalars used in noise generation. */
		inline void SetScalars(_In_ float scale, _In_ float persistance, _In_ float lacunarity)
		{
			s = scale;
			p = persistance;
			l = lacunarity;
		}

	private:
		PerlinNoise perlin;
		byte w, d;
		size_t o;
		float s, p, l;

		static Vector3 SurfaceNormal(Terrain *vertices, uint16 a, uint16 b, uint16 c);

		void EmplaceData(Terrain *vertices, uint16 *indices, Vector2 pos);
	};
}