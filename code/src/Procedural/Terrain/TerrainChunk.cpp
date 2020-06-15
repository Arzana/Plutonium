#include "Procedural/Terrain/TerrainChunk.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Graphics/Models/ShapeCreator.h"

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
			: result(result), pool(pool), layout(layout), noise(noise), offset(offset)
		{}

		Result Execute(void) final
		{
			/* Allocate the displacement GPU image. */
			LogicalDevice &device = result->fetcher->GetDevice();
			result->displacement = new Image(device, ImageCreateInfo{ ImageType::Image2D, Format::R32_SFLOAT, Extent3D(meshSize, meshSize, 1), 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::Storage | ImageUsageFlag::TransferDst });
			result->view = new ImageView(*result->displacement, ImageViewType::Image2D, ImageAspectFlag::Color);

			/* Allocate a single staging buffer for both the displacement image and the mesh. */
			const bool canPatch = device.GetPhysicalDevice().GetEnabledFeatures().TessellationShader;
			const size_t meshBufferSize = canPatch ? ShapeCreator::GetPatchPlaneBufferSize(meshSize) : ShapeCreator::GetPlaneBufferSize(meshSize);
			const uint32 vrtxSize = ShapeCreator::GetPlaneVertexSize(meshSize);
			const size_t stagingBufferSize = meshBufferSize + sqr(meshSize) * sizeof(float);
			stagingBuffer = new StagingBuffer(device, stagingBufferSize);

			/* We don't want to map/unmap/flush multiple times so we handle memory ourselves. */
			stagingBuffer->BeginMemoryTransfer();
			Patched3D *vertex = reinterpret_cast<Patched3D*>(stagingBuffer->GetHostMemory());
			float *pixel = reinterpret_cast<float*>(reinterpret_cast<byte*>(stagingBuffer->GetHostMemory()) + meshBufferSize);

			/* Set the position and uv's for the mesh. */
			Mesh mesh = canPatch ? ShapeCreator::PatchPlane(*stagingBuffer, meshSize, false) : ShapeCreator::LodPlane(*stagingBuffer, meshSize, false);
			result->mesh.Initialize(device, static_cast<uint32>(meshBufferSize - vrtxSize), vrtxSize, std::move(mesh));

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

					pixel[i++] = h;
					minH = min(minH, h);
					maxH = max(maxH, h);
				}
			}

			/* Normalize the height. */
			minH *= iMaxPerlin;
			maxH *= iMaxPerlin;
			for (i = 0; i < sqr(meshSize); i++)
			{
				pixel[i] *= iMaxPerlin;
			}

			/* Calculate the normals for the mesh and heightmap. */
			HeightMap tmpCollider{ meshSize, meshScale, true };
			i = 0;
			for (int32 y = 0; y < meshSize; y++)
			{
				for (int32 x = 0; x < meshSize; x++, i++)
				{
					const float h0 = pixel[rectify(y - 1) * meshSize + x] * heightScale;
					const float h1 = pixel[y * meshSize + rectify(x - 1)] * heightScale;
					const float h2 = pixel[y * meshSize + min(x + 1, meshBound)] * heightScale;
					const float h3 = pixel[min(y + 1, meshBound) * meshSize + x] * heightScale;
					Vector3 n{ h1 - h2, 2.0f, h0 - h3 };
					n.Normalize();

					vertex[i].Normal = n;
					tmpCollider.SetHeightAndNormal(x, y, pixel[i] * heightScale, n);
				}
			}

			stagingBuffer->EndMemoryTransfer();
			const ImageSubresourceRange range = result->displacement->GetFullRange(ImageAspectFlag::Color);

			/* Stage the mesh and displacement texture to the GPU. */
			cmd.Initialize(device, device.GetGraphicsQueueFamily());
			cmd.Begin();
			cmd.MemoryBarrier(result->mesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, AccessFlag::TransferWrite);
			cmd.MemoryBarrier(*result->displacement, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, range);
			cmd.CopyBuffer(*stagingBuffer, result->mesh.GetBuffer(), BufferCopy{ 0, 0, meshBufferSize });
			cmd.CopyBuffer(*stagingBuffer, *result->displacement, BufferImageCopy{ meshBufferSize, result->displacement->GetExtent() });
			cmd.MemoryBarrier(result->mesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
			cmd.MemoryBarrier(*result->displacement, PipelineStageFlag::Transfer, canPatch ? PipelineStageFlag::TessellationControlShader : PipelineStageFlag::VertexShader, ImageLayout::General, AccessFlag::ShaderRead, range);
			cmd.End();
			device.GetGraphicsQueue(0).Submit(cmd);

			/* The terrain is rendered fro the center, so we need to add an offset. */
			result->pos = Vector3(offset.X, 0.0f, offset.Y) * meshScale * meshBound;
			const float halfSize = meshBound * 0.5f * meshScale;

			/* Initialize the material. */
			result->material = new Terrain(pool, layout);
			result->material->SetHeight(*result->view);
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
				Collider collider{ result->bb, CollisionShapes::HeightMap, &tmpCollider };
				PhysicalObject obj{ result->pos, Quaternion{}, collider };
				result->hcollider = result->world->AddStatic(obj);
			}

			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			delete stagingBuffer;
			result->usable = true;
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			return cmd.CanBegin() && result->textures->IsUsable() && result->albedoMask->IsUsable();
		}

	private:
		TerrainChunk *result;
		DescriptorPool &pool;
		const DescriptorSetLayout &layout;
		PerlinNoise &noise;
		Vector2 offset;

		StagingBuffer *stagingBuffer;
		SingleUseCommandBuffer cmd;
	};
}

Pu::TerrainChunk::TerrainChunk(AssetFetcher & fetcher, PhysicalWorld * world)
	: generated(false), fetcher(&fetcher), hcollider(PhysicsNullHandle),
	displacement(nullptr), textures(nullptr), material(nullptr), usable(false),
	world(world)
{}

Pu::TerrainChunk::TerrainChunk(TerrainChunk && value)
	: generated(value.generated), fetcher(value.fetcher),
	hcollider(value.hcollider), mesh(std::move(value.mesh)),
	displacement(value.displacement), textures(value.textures),
	material(value.material), albedoMask(value.albedoMask),
	usable(value.usable.load()), world(value.world)
{
	value.displacement = nullptr;
	value.textures = nullptr;
	value.material = nullptr;
	value.albedoMask = nullptr;
	value.usable = false;
}

Pu::TerrainChunk & Pu::TerrainChunk::operator=(TerrainChunk && other)
{
	if (this != &other)
	{
		Destroy();

		generated = other.generated;
		usable = other.usable.load();
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
		other.usable = false;
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
	fetcher->GetScheduler().Spawn(*task);
}

void Pu::TerrainChunk::Destroy(void)
{
	/* Remove the terrain chunk from the physical world. */
	if (hcollider != PhysicsNullHandle) world->Destroy(hcollider);

	if (material) delete material;
	if (textures) fetcher->Release(*textures);
	if (albedoMask) fetcher->Release(*albedoMask);

	/* We need to delete all aspects of the texture, because we allocated them ourselves. */
	if (displacement)
	{
		delete view;
		delete displacement;
	}
}