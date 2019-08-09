#pragma once
#include "Graphics/Models/Mesh.h"
#include "Environment/Terrain/Lithosphere.h"

namespace Pu
{
	struct Terrain;

	/* Defines a helper object for generating terrains easily. */
	class TerrainCreator
	{
	public:
		/* Initializes a new instance of a terrain generator with the specified lithosphere. */
		TerrainCreator(_In_ const Lithosphere &lithosphere);
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
		_Check_return_ Mesh Generate(_In_ Buffer &src, _In_ const Buffer &dst);

	private:
		const Lithosphere &lithosphere;

		static Vector3 SurfaceNormal(Terrain *vertices, uint32 a, uint32 b, uint32 c);

		void EmplaceData(Terrain *vertices, uint32 *indices);
	};
}