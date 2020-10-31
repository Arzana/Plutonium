#pragma once
#include "Content/AssetFetcher.h"
#include "Graphics/Models/Terrain.h"
#include "Physics/Systems/PhysicalWorld.h"

namespace Pu
{
	class TextureStorage2D;

	/* Defines an object capable of generating terrain chunks. */
	class ChunkGenerator
	{
	public:
		/* Initializes a new instance of a chunk generator. */
		ChunkGenerator(_In_ AssetFetcher &fetcher, _In_ PhysicalWorld &world, _In_ uint32 maxChunks);
		ChunkGenerator(_In_ const ChunkGenerator&) = delete;
		/* Move constructor. */
		ChunkGenerator(_In_ ChunkGenerator &&value);
		/* Releases the resources allocated by the chunk generator. */
		~ChunkGenerator(void)
		{
			Destroy();
		}

		_Check_return_ ChunkGenerator& operator =(_In_ const ChunkGenerator&) = delete;
		/* Move assginment. */
		_Check_return_ ChunkGenerator& operator =(_In_ ChunkGenerator &&other);

		/* Creates a new terrain chunk at the specified chunk offset. */
		void Create(_In_ Vector2 offset);
		/* Destroys the terrain chunk at the specified chunk offset. */
		void Destroy(_In_ Vector2 offset);
		/* Clears all chunks from the generator. */
		void Clear(void);

	private:
		friend class CreateChunkTask;

		AssetFetcher *fetcher;
		PhysicalWorld *world;
		MeshCollection mesh;
		Texture2DArray *textures;
		std::mutex lock;

		Buffer *perlinPermutations;
		ShaderProgram *perlinShader;
		ComputePipeline *perlinPipe;
		DescriptorPool *perlinDescPool;
		DescriptorSet *perlinConstDescSet;

		uint32 maxChunks;
		vector<Vector2> offsets;
		vector<TextureStorage2D*> displacements;
		vector<Terrain*> materials;

		void ComputePostLink(ShaderProgram&);
		void Destroy(void);
	};
}