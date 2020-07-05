#pragma once
#include "Content/AssetFetcher.h"
#include "Graphics/Models/Terrain.h"
#include "Core/Math/PerlinNoise.h"
#include "Core/Math/HeightMap.h"
#include "Physics/Systems/PhysicalWorld.h"

namespace Pu
{
	/* Defines a single terrain chunk that can be created on the fly. */
	class TerrainChunk
	{
	public:
		/* Initializes an empty instance of a terrain chunk. */
		TerrainChunk(_In_ AssetFetcher &fetcher, _In_ PhysicalWorld *physics);
		TerrainChunk(_In_ const TerrainChunk&) = delete;
		/* Move constructor. */
		TerrainChunk(_In_ TerrainChunk &&value);
		/* Releases the resources allocated by the terrain chunk. */
		~TerrainChunk(void)
		{
			Destroy();
		}

		_Check_return_ TerrainChunk& operator =(_In_ const TerrainChunk&) = delete;
		/* Move assignment. */
		_Check_return_ TerrainChunk& operator =(_In_ TerrainChunk &&other);

		/* Generates the mesh for this terrain chunk. */
		void Initialize(_In_ const wstring &mask, _In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout, _In_ PerlinNoise &noise, _In_ Vector2 offset, _In_ const vector<wstring> &albedos);

		/* Gets whether this chunk has been generated. */
		_Check_return_ inline bool IsGenerated(void) const
		{
			return generated;
		}

		/* Gets whether this chunk is ready to be used. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return usable.load();
		}

		/* Gets the meshes associated with this terrain chunk. */
		_Check_return_ inline const MeshCollection& GetMeshes(void) const
		{
			return mesh;
		}

		/* Gets the material associated with this terrain chunk. */
		_Check_return_ inline const Terrain& GetMaterial(void) const
		{
			return *material;
		}

		/* Gets the bounding box of this terrain chunk. */
		_Check_return_ inline const AABB& GetBoundingBox(void) const
		{
			return bb;
		}

		/* Gets the transform of this chunk in the physical world. */
		_Check_return_ inline Vector3 GetPosition(void) const
		{
			return pos;
		}

	private:
		friend class ChunkCreator;

		bool generated;
		std::atomic_bool usable;
		AssetFetcher *fetcher;
		AABB bb;

		PhysicalWorld *world;
		PhysicsHandle hcollider;
		Vector3 pos;

		MeshCollection mesh;
		Texture2D *albedoMask;
		Image *displacement;
		ImageView *view;
		Texture2DArray *textures;
		Terrain *material;

		void Destroy(void);
	};
}