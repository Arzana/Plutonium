#include "Procedural/Terrain/TerrainChunk.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Graphics/Models/ShapeCreator.h"

const Pu::uint16 meshSize = 32;
const float meshScale = 10.0f;
const float displacement = 8.0f;

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
			Image *img = new Image(device, ImageCreateInfo{ ImageType::Image2D, Format::R32_SFLOAT, Extent3D(meshSize, meshSize, 1), 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::Sampled | ImageUsageFlag::TransferDst });
			result->displacement = new Texture2D(*img, result->fetcher->FetchSampler(SamplerCreateInfo{ Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToEdge }));

			/* Allocate a single staging buffer for both the displacement image and the mesh. */
			const size_t meshBufferSize = ShapeCreator::GetPatchPlaneBufferSize(meshSize);
			const uint32 vrtxSize = ShapeCreator::GetPatchPlaneVertexSize(meshSize);
			const size_t stagingBufferSize = meshBufferSize + sqr(meshSize) * sizeof(float);
			stagingBuffer = new StagingBuffer(device, stagingBufferSize);

			/* We don't want to map/unmap/flush multiple times so we handle memory ourselves. */
			stagingBuffer->BeginMemoryTransfer();
			Patched3D *vertex = reinterpret_cast<Patched3D*>(stagingBuffer->GetHostMemory());
			float *pixel = reinterpret_cast<float*>(reinterpret_cast<byte*>(stagingBuffer->GetHostMemory()) + meshBufferSize);

			/* Set the position and uv's for the mesh. */
			Mesh mesh = ShapeCreator::PatchPlane(*stagingBuffer, meshSize, false);
			result->mesh.Initialize(device, static_cast<uint32>(meshBufferSize - vrtxSize), vrtxSize, std::move(mesh));

			/* Generate the unnormalized displacement map. */
			const float step = recip(meshSize);
			float mi = maxv<float>(), ma = minv<float>();
			size_t i = 0;
			for (float y = 0; y < 1.0f; y += step)
			{
				for (float x = 0; x < 1.0f; x += step)
				{
					const float h = noise.NormalizedScale(offset.X + x, offset.Y + y, 4, 0.5f, 2.0f);
					pixel[i++] = h;

					mi = min(mi, h);
					ma = max(ma, h);
				}
			}

			/* Normalize the height. */
			for (i = 0; i < sqr(meshSize); i++)
			{
				pixel[i] = ilerp(mi, ma, pixel[i]);
			}

			/* Calculate the normals for the mesh and heightmap. */
			i = 0;
			for (int32 y = 0; y < meshSize; y++)
			{
				for (int32 x = 0; x < meshSize; x++, i++)
				{
					float h0 = pixel[rectify(y - 1) * meshSize + x];
					float h1 = pixel[y * meshSize + rectify(x - 1)];
					float h2 = pixel[y * meshSize + max(x + 1, meshSize - 1)];
					float h3 = pixel[max(y + 1, meshSize - 1) * meshSize + x];
					Vector3 n{ h1 - h2, 2.0f, h0 - h3 };
					n.Normalize();

					vertex[i].Normal = n;
					result->collider.SetHeightAndNormal(x, y, pixel[i], n);
				}
			}

			stagingBuffer->EndMemoryTransfer();

			/* Stage the mesh and displacement texture to the GPU. */
			cmd.Initialize(device, device.GetGraphicsQueueFamily());
			cmd.Begin();
			cmd.MemoryBarrier(result->mesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, AccessFlag::TransferWrite);
			cmd.MemoryBarrier(*img, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, result->displacement->GetFullRange());
			cmd.CopyBuffer(*stagingBuffer, result->mesh.GetBuffer(), BufferCopy{ 0, 0, meshBufferSize });
			cmd.CopyBuffer(*stagingBuffer, *img, BufferImageCopy{ meshBufferSize, img->GetExtent() });
			cmd.MemoryBarrier(result->mesh.GetBuffer(), PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);
			cmd.MemoryBarrier(*img, PipelineStageFlag::Transfer, PipelineStageFlag::TessellationControlShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, result->displacement->GetFullRange());
			cmd.End();
			device.GetGraphicsQueue(0).Submit(cmd);

			/* The terrain is rendered fro the center, so we need to add an offset. */
			const Vector3 pos = Vector3(offset.X, 0.0f, offset.Y) * meshScale * (meshSize - 1);
			const float halfSize = meshSize * 0.5f * meshScale;

			/* Initialize the material. */
			result->material = new Terrain(pool, layout);
			result->material->SetHeight(*result->displacement);
			result->material->SetMask(*result->albedoMask);
			result->material->SetTextures(*result->textures);
			result->material->SetDisplacement(displacement);
			result->material->SetScale(meshScale);
			result->material->SetPatchSize(meshSize);
			result->material->SetPosition(pos + Vector3(halfSize, 0.0f, halfSize));

			result->bb.LowerBound = pos;
			result->bb.UpperBound = pos + Vector3(meshScale * meshSize, meshScale * displacement, meshScale * meshSize);
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

Pu::TerrainChunk::TerrainChunk(AssetFetcher & fetcher)
	: generated(false), fetcher(&fetcher), collider(meshSize, meshScale, true),
	displacement(nullptr), textures(nullptr), material(nullptr), usable(false)
{}

Pu::TerrainChunk::TerrainChunk(TerrainChunk && value)
	: generated(value.generated), fetcher(value.fetcher),
	collider(std::move(value.collider)), mesh(std::move(value.mesh)),
	displacement(value.displacement), textures(value.textures),
	material(value.material), albedoMask(value.albedoMask),
	usable(value.usable.load())
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
		collider = std::move(other.collider);
		mesh = std::move(other.mesh);
		displacement = other.displacement;
		textures = other.textures;
		material = other.material;
		albedoMask = other.albedoMask;

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
	if (material) delete material;
	if (textures) fetcher->Release(*textures);
	if (albedoMask) fetcher->Release(*albedoMask);

	/* We need to delete all aspects of the texture, because we allocated them ourselves. */
	if (displacement)
	{
		fetcher->Release(displacement->GetSampler());
		delete &displacement->GetImage();
		delete displacement;
	}
}