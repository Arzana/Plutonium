#include "Environment/Terrain/TerrainCreator.h"
#include "Graphics/VertexLayouts/Terrain.h"
#include "Core/Math/Interpolation.h"

Pu::TerrainCreator::TerrainCreator(const Lithosphere & lithosphere)
	: lithosphere(lithosphere)
{}

size_t Pu::TerrainCreator::GetBufferSize(void) const
{
	const size_t x = lithosphere.GetSide();
	return (lithosphere.GetSize() * sizeof(Terrain)) + (sqr(x - 1) * 6 * sizeof(uint32));
}

Pu::Mesh Pu::TerrainCreator::Generate(Buffer & src, const Buffer & dst)
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

	const size_t vrtxCount = lithosphere.GetSize();
	const size_t idxCount = sqr(lithosphere.GetSide() - 1) * 6;

	/* Begin the memory transfer. */
	src.BeginMemoryTransfer();
	Terrain *vertices = reinterpret_cast<Terrain*>(src.GetHostMemory());
	uint32 *indices = reinterpret_cast<uint32*>(vertices + vrtxCount);

	/* Actually generate the terrain. */
	EmplaceData(vertices, indices);

	/* Finalize the memory and create the mesh. */
	src.EndMemoryTransfer();
	return Mesh(dst, vrtxCount * sizeof(Terrain), idxCount * sizeof(uint32), sizeof(Terrain), sizeof(uint32), IndexType::UInt32);
}

Pu::Vector3 Pu::TerrainCreator::SurfaceNormal(Terrain * vertices, uint32 a, uint32 b, uint32 c)
{
	const Vector3 p1 = vertices[a].Position;
	const Vector3 p2 = vertices[b].Position;
	const Vector3 p3 = vertices[c].Position;

	const Vector3 s1 = p2 - p1;
	const Vector3 s2 = p3 - p1;
	return cross(s1, s2).Normalize();
}

void Pu::TerrainCreator::EmplaceData(Terrain * vertices, uint32 * indices)
{
	/* Pre-define some constants to speed things up. */
	const uint32 w = static_cast<uint32>(lithosphere.GetSide());
	const float tlx = (w - 1) / -2.0f;
	const float tlz = (w - 1) / 2.0f;
	const float iw = 1.0f / w;
	const float id = 1.0f / w;

	const float *heightMap = lithosphere.GetTopography();
	const size_t *idxMap = lithosphere.GetPlateIDs();

	/* Loop through the grid that is our terrain. */
	uint32 i = 0;
	for (float z = 0; z < w; z++)
	{
		for (float x = 0; x < w; x++, vertices++, i++)
		{
			const size_t j = static_cast<size_t>(z * w + x);

			/* Generate a perlin value for the terrain height and set the geometric values. */
			vertices->Position = Vector3(tlx + x, heightMap[j], tlz - z);
			vertices->Normal = Vector3();
			vertices->PlateId = static_cast<uint32>(idxMap[j]);

			/* Only add a quad if we're not on the right or down edge. */
			if (x < w - 1 && z < w - 1)
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
	vertices -= lithosphere.GetSize();
	indices -= sqr(w - 1) * 6;
	for (size_t j = 0; j < sqr(w - 1) * 6; j += 3)
	{
		const uint32 a = indices[j];
		const uint32 b = indices[j + 1];
		const uint32 c = indices[j + 2];

		const Vector3 normal = SurfaceNormal(vertices, a, b, c);
		vertices[a].Normal += normal;
		vertices[b].Normal += normal;
		vertices[c].Normal += normal;
	}

	/* Normalize the summed surface normals to get the vertex normals. */
	for (i = 0; i < sqr(w); i++)
	{
		vertices[i].Normal.Normalize();
	}
}