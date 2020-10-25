#include "Procedural/Terrain/TerrainChunk.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Physics/Systems/PhysicalWorld.h"
#include "Graphics/Models/ShapeCreator.h"
#include "Streams/RuntimeConfig.h"

const Pu::uint16 meshSize = 64;
const Pu::uint16 meshBound = meshSize - 1;
const Pu::uint32 octaves = 4;
const float persistance = 0.5f;
const float lacunarity = 2.0f;
const float meshScale = 1.0f;
const float displacement = 32.0f;
const float heightScale = meshScale * displacement;

namespace Pu
{
	class ChunkCreator
		: public Task
	{
	public:
		ChunkCreator(TerrainChunk *result, DescriptorPool &pool, const DescriptorSetLayout &layout, PerlinNoise &noise, Vector2 offset)
			: Task("Generate Chunk"), result(result), pool(pool), layout(layout), noise(noise), offset(offset)
		{}

		Result Execute(void) final
		{
			/* Precalculate often used values. */
			LogicalDevice &device = result->fetcher->GetDevice();
			pixels = reinterpret_cast<float*>(malloc(sqr(meshSize) * sizeof(float)));
			const float iMaxPerlin = recip(PerlinNoise::Max(octaves, persistance));
			const float step = recip(meshBound);
			const float start = step * 0.5f;
			float minH = maxv<float>(), maxH = minv<float>();
			size_t i = 0;

			/* Generate the unnormalized displacement map. */
			for (float y = 0.0f; y <= 1.0f; y += step)
			{
				for (float x = 0.0f; x <= 1.0f; x += step)
				{
					const float h = noise.NormalizedScale(offset.X + x, offset.Y + y, octaves, persistance, lacunarity);
					pixels[i++] = h;

					minH = min(minH, h);
					maxH = max(maxH, h);
				}
			}

			/* Normalize the height. */
			minH *= iMaxPerlin;
			maxH *= iMaxPerlin;
			for (i = 0; i < sqr(meshSize); i++) pixels[i] *= iMaxPerlin;
			i = 0;

			/* Allocate the staging buffer for the mesh (either tessellated or normal). */
			const bool shouldPatch = RuntimeConfig::QueryBool(L"TessellationEnabled");
			const size_t meshBufferSize = shouldPatch ? ShapeCreator::GetPatchPlaneBufferSize(meshSize) : ShapeCreator::GetPlaneBufferSize(meshSize);
			const uint32 vrtxSize = ShapeCreator::GetPlaneVertexSize(meshSize);
			stagingBuffer = new StagingBuffer(device, meshBufferSize);

			/* We need to set the normals ourselves so handle mapping the host ourselves. */
			stagingBuffer->BeginMemoryTransfer();
			Patched3D *vrtx = reinterpret_cast<Patched3D*>(stagingBuffer->GetHostMemory());

			/* Load the desired mesh into the staging buffer and finalize the mesh collection. */
			Mesh mesh = shouldPatch ? ShapeCreator::PatchPlane(*stagingBuffer, meshSize, false) : ShapeCreator::LodPlane(*stagingBuffer, meshSize, false);
			result->mesh.AddMesh(mesh, 0u, vrtxSize, vrtxSize, static_cast<uint32>(meshBufferSize - vrtxSize));
			result->mesh.Finalize(device, meshBufferSize);

			/* Calculate the normals for the mesh and heightmap. */
			HeightMap heightMap{ meshSize, meshScale, true };
			for (int32 y = 0; y < meshSize; y++)
			{
				for (int32 x = 0; x < meshSize; x++, i++)
				{
					/* Sample four heights from the adjacent pixels. */
					const float h0 = pixels[rectify(y - 1) * meshSize + x] * heightScale;
					const float h1 = pixels[y * meshSize + rectify(x - 1)] * heightScale;
					const float h2 = pixels[y * meshSize + min(x + 1, meshBound)] * heightScale;
					const float h3 = pixels[min(y + 1, meshBound) * meshSize + x] * heightScale;

					/* Calculate the normal and pass it to the mesh and collider. */
					const Vector3 n = normalize(h1 - h2, 2.0f, h0 - h3);
					vrtx[i].Normal = n;
					heightMap.SetHeightAndNormal(x, y, pixels[i] * heightScale, n);
				}
			}

			/* Stage the mesh to the GPU. */
			stagingBuffer->EndMemoryTransfer();
			result->fetcher->GetLoader().StageBuffer(*stagingBuffer, result->mesh.GetBuffer(), PipelineStageFlags::VertexShader, AccessFlags::VertexAttributeRead, L"TerrainChunkPlane");

			/* Stage the displacement texture to the GPU, random name is needed in order not to replace textures. */
			const SamplerCreateInfo samplerInfo{ Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToEdge };
			result->displacement = &result->fetcher->CreateTexture2D("TerrainChunkDisplacement", pixels, meshSize, meshSize, Format::R32_SFLOAT, samplerInfo);

			/* The terrain is rendered fro the center, so we need to add an offset. */
			result->pos = Vector3(offset.X, 0.0f, offset.Y) * meshScale * meshBound;
			const float halfSize = meshBound * 0.5f * meshScale;

			/* Initialize the material. */
			result->material = new Terrain(pool, layout);
			result->material->SetHeight(*result->displacement);
			result->material->SetMask(*result->albedoMask);
			result->material->SetTextures(*result->textures);
			result->material->SetDisplacement(displacement);
			result->material->SetScale(meshScale);
			result->material->SetPatchSize(meshSize);
			result->material->SetPosition(result->pos + Vector3(halfSize, 0.0f, halfSize));

			result->bb.LowerBound = Vector3(0.0f, minH * heightScale, 0.0f);
			result->bb.UpperBound = Vector3(meshScale * meshBound, maxH * heightScale, meshScale * meshBound);

			/* Add the heightmap to the physical world if needed. */
			if (result->world)
			{
				Collider collider{ result->bb, CollisionShapes::HeightMap, &heightMap };
				PhysicalObject obj{ result->pos, Quaternion{}, collider };
				obj.Properties = create_physics_handle(PhysicsType::Material, 1ull);
				result->hcollider = result->world->AddStatic(obj, *result);
			}

			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			result->MarkAsLoaded(false, L"TerrainChunk");

			/* Free all the used resources. */
			free(pixels);
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			/* Wait till the mesh and displacement are staged. */
			return result->mesh.GetBuffer().IsLoaded()
				&& result->displacement->IsUsable()
				&& result->textures->IsUsable()
				&& result->albedoMask->IsUsable();
		}

	private:
		TerrainChunk *result;
		DescriptorPool &pool;
		const DescriptorSetLayout &layout;
		PerlinNoise &noise;
		Vector2 offset;

		StagingBuffer *stagingBuffer;
		float *pixels;
	};
}

Pu::TerrainChunk::TerrainChunk(AssetFetcher & fetcher, PhysicalWorld * world)
	: Asset(false), generated(false), fetcher(&fetcher), hcollider(PhysicsNullHandle),
	displacement(nullptr), textures(nullptr), material(nullptr), world(world)
{
	SetHash(random(0u, 0xFFFFFFFE));
}

Pu::TerrainChunk::TerrainChunk(TerrainChunk && value)
	: Asset(std::move(value)), generated(value.generated), 
	fetcher(value.fetcher), hcollider(value.hcollider), 
	mesh(std::move(value.mesh)), displacement(value.displacement), 
	textures(value.textures), material(value.material),
	albedoMask(value.albedoMask), world(value.world)
{
	value.displacement = nullptr;
	value.textures = nullptr;
	value.material = nullptr;
	value.albedoMask = nullptr;
}

Pu::TerrainChunk & Pu::TerrainChunk::operator=(TerrainChunk && other)
{
	if (this != &other)
	{
		Destroy();
		Asset::operator=(std::move(other));

		generated = other.generated;
		fetcher = other.fetcher;
		hcollider = other.hcollider;
		mesh = std::move(other.mesh);
		displacement = other.displacement;
		textures = other.textures;
		material = other.material;
		albedoMask = other.albedoMask;
		world = other.world;

		other.displacement = nullptr;
		other.textures = nullptr;
		other.material = nullptr;
		other.albedoMask = nullptr;
	}

	return *this;
}

void Pu::TerrainChunk::Initialize(const wstring & mask, DescriptorPool & pool, const DescriptorSetLayout & layout, PerlinNoise & noise, Vector2 offset, const vector<wstring>& albedos)
{
	/* Make sure initialize isn't called multiple times. */
	if (generated) return;
	generated = true;

	/* We load the textures immediately, because this is relatively inexpensive. */
	textures = &fetcher->FetchTexture2DArray("Terrain Textures", SamplerCreateInfo(), true, albedos);
	albedoMask = &fetcher->FetchTexture2D(mask, SamplerCreateInfo{ Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToEdge }, false, 1);

	/* The task will self delete so we can use spawn it and stop caring about it. */
	ChunkCreator *task = new ChunkCreator(this, pool, layout, noise, offset);
	TaskScheduler::Spawn(*task);
}

Pu::Asset & Pu::TerrainChunk::Duplicate(AssetCache&)
{
	Log::Fatal("TerrainChunk cannot be duplicated!");
	return *this;
}

void Pu::TerrainChunk::Destroy(void)
{
	/* Remove the terrain chunk from the physical world. */
	if (hcollider != PhysicsNullHandle) world->Destroy(hcollider);

	if (material) delete material;
	if (textures) fetcher->Release(*textures);
	if (albedoMask) fetcher->Release(*albedoMask);
	if (displacement) fetcher->Release(*displacement);
}