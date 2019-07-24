#include "Environment/Terrain/TerrainCreator.h"
#include "Graphics/VertexLayouts/Terrain.h"
#include "Core/Math/Interpolation.h"

Pu::TerrainCreator::TerrainCreator(const PerlinNoise & noise)
	: perlin(noise), w(65), d(65), o(5), s(0.1234f), p(0.5f), l(2.0f)
{}

size_t Pu::TerrainCreator::GetBufferSize(void) const
{
	return (w * d * sizeof(Terrain)) + ((w - 1) * (d - 1) * 6 * sizeof(uint16));
}

Pu::Mesh Pu::TerrainCreator::Generate(Buffer & src, const Buffer & dst, Vector2 position)
{
	/* Check if the input buffer is correct (only on debug). */
#ifdef _DEBUG
	if (src.GetSize() < GetBufferSize())
	{
		Log::Error("Source buffer is not large engough to accomodate the terrain!");
		return Mesh();
	}

	if (!src.IsHostAccessible())
	{
		Log::Error("Source buffer is not accessible by the CPU!");
		return Mesh();
	}
#endif

	/* Begin the memory transfer. */
	src.BeginMemoryTransfer();
	Terrain *vertices = reinterpret_cast<Terrain*>(src.GetHostMemory());
	uint16 *indices = reinterpret_cast<uint16*>(vertices + w * d);

	/* Actually generate the terrain. */
	EmplaceData(vertices, indices, position);

	/*
	Finalize the memory and create the mesh.
	Our index type is set to 16 bts so our maximum mapsize can only be 256x256 but that should be plenty.
	*/
	src.EndMemoryTransfer();
	return Mesh(dst, w * d * sizeof(Terrain), (w - 1) * (d - 1) * 6 * sizeof(uint16), sizeof(Terrain), sizeof(uint16), IndexType::UInt16);
}

Pu::Vector3 Pu::TerrainCreator::SurfaceNormal(Terrain * vertices, uint16 a, uint16 b, uint16 c)
{
	const Vector3 p1 = vertices[a].Position;
	const Vector3 p2 = vertices[b].Position;
	const Vector3 p3 = vertices[c].Position;

	const Vector3 s1 = p2 - p1;
	const Vector3 s2 = p3 - p1;
	return cross(s1, s2).Normalize();
}

#include "Core/Math/SquareDiamondNoise.h"

void Pu::TerrainCreator::EmplaceData(Terrain * vertices, uint16 * indices, Vector2 pos)
{
	/* Pre-define some constants to speed things up. */
	const float tlx = (w - 1) / -2.0f;
	const float tlz = (d - 1) / 2.0f;
	const float iw = 1.0f / w;
	const float id = 1.0f / d;

	SquareDiamondNoise noise{};
	const float *heightMap = noise.GenerateNormalized();

	/* Loop through the grid that is our terrain. */
	uint16 i = 0;
	for (float z = 0; z < d; z++)
	{
		for (float x = 0; x < w; x++, vertices++, i++)
		{
			/* Generate a perlin value for the terrain height and set the geometric values. */
			const float y = heightMap[static_cast<uint32>(z * w + x)];
			vertices->Position = Vector3(tlx + x, y, tlz - z);
			vertices->Normal = Vector3();

			/* Only add a quad if we're not on the right or down edge. */
			if (x < w - 1 && z < d - 1)
			{
				indices[0] = i;
				indices[2] = i + w + 1;
				indices[1] = i + w;
				indices[3] = i + w + 1;
				indices[5] = i;
				indices[4] = i + 1;

				indices += 6;
			}
		}
	}

	/* Calculate the surface normals. */
	vertices -= w * d;
	indices -= (w - 1) * (d - 1) * 6;
	for (size_t j = 0; j < (w - 1) * (d - 1) * 6; j += 3)
	{
		const uint16 a = indices[j];
		const uint16 b = indices[j + 1];
		const uint16 c = indices[j + 2];

		const Vector3 normal = SurfaceNormal(vertices, a, b, c);
		vertices[a].Normal += normal;
		vertices[b].Normal += normal;
		vertices[c].Normal += normal;
	}

	/* Normalize the summed surface normals to get the vertex normals. */
	for (i = 0; i < w * d; i++)
	{
		vertices[i].Normal.Normalize();
	}
}