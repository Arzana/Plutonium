#include "Procedural/Terrain/ChunkGenerator.h"
#include "Graphics/Resources/SingleUseCommandBuffer.h"
#include "Graphics/Textures/TextureStorage2D.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Models/ShapeCreator.h"
#include "Core/Threading/PuThread.h"
#include "Streams/RuntimeConfig.h"

using namespace Pu;

class BufferLoadTask
	: public Task
{
public:
	BufferLoadTask(AssetLoader &loader, MeshCollection &meshes, Buffer *&permutations)
		: Task("Chunk Generator Buffer Initialize"), loader(&loader),
		meshes(&meshes), permutations(&permutations)
	{}

	Result Execute(void) final
	{
		/* Calculate all the buffer ranges. */
		constexpr size_t PERMUTATIONS_BUFFER_SIZE = 512 * sizeof(uint32);
		const uint16 chunkSize = static_cast<uint16>(RuntimeConfig::QueryInt(L"ChunkSize", 64));
		const size_t tessBufferSize = ShapeCreator::GetPatchPlaneBufferSize(chunkSize);
		const size_t lodBufferSize = ShapeCreator::GetPlaneBufferSize(chunkSize);
		const uint32 meshVrtxSize = ShapeCreator::GetPlaneVertexSize(chunkSize);

		/* Allocate the buffers (staging buffers will be deleted by the stage task). */
		StagingBuffer *meshStagingBuffer = new StagingBuffer(loader->GetDevice(), tessBufferSize + lodBufferSize);
		StagingBuffer *perlinStagingBuffer = new StagingBuffer(loader->GetDevice(), PERMUTATIONS_BUFFER_SIZE);
		*permutations = new Buffer(loader->GetDevice(), PERMUTATIONS_BUFFER_SIZE, BufferUsageFlags::TransferDst | BufferUsageFlags::StorageBuffer, MemoryPropertyFlags::DeviceLocal);

		/* Load the mesh data into the staging buffer. */
		meshStagingBuffer->BeginMemoryTransfer();
		meshes->AddMesh(ShapeCreator::PatchPlane(*meshStagingBuffer, chunkSize, false), 0u, meshVrtxSize, meshVrtxSize, static_cast<uint32>(tessBufferSize - meshVrtxSize));
		meshes->AddMesh(ShapeCreator::LodPlane(*meshStagingBuffer, chunkSize, false, tessBufferSize), tessBufferSize, meshVrtxSize, tessBufferSize + meshVrtxSize, static_cast<uint32>(lodBufferSize - meshVrtxSize));
		meshes->Finalize(loader->GetDevice(), tessBufferSize + lodBufferSize);
		meshStagingBuffer->EndMemoryTransfer();
		
		/* Load the permutation data into the staging buffer. */
		perlinStagingBuffer->BeginMemoryTransfer();
		uint32 *mem = reinterpret_cast<uint32*>(perlinStagingBuffer->GetHostMemory());
		for (uint32 i = 0; i < 256; i++) mem[i] = i;
		std::shuffle(mem, mem + 256, std::default_random_engine{ static_cast<uint32>(time(nullptr)) & maxv<uint32>() });
		memcpy(mem + 256, mem, 256 * sizeof(uint32));
		perlinStagingBuffer->EndMemoryTransfer();

		/* Stage both the buffers to the GPU. */
		loader->StageBuffer(*meshStagingBuffer, meshes->GetBuffer(), PipelineStageFlags::VertexShader, AccessFlags::VertexAttributeRead, L"Chunk Generator Meshes");
		loader->StageBuffer(*perlinStagingBuffer, **permutations, PipelineStageFlags::ComputeShader, AccessFlags::ShaderRead, L"Chunk Generator Perlin Permutations");
		return Result::AutoDelete();
	}

private:
	AssetLoader *loader;
	MeshCollection *meshes;
	Buffer **permutations;
};

namespace Pu
{
	class CreateChunkTask
		: public Task
	{
	public:
		CreateChunkTask(ChunkGenerator *parent, Vector2 offset)
			: Task("Generate Chunk"), parent(parent), offset(offset)
		{}

		Result Execute(void) final
		{
			/* Initialize the command buffer. */
			LogicalDevice &device = parent->fetcher->GetDevice();
			cmdBuffer.Initialize(device, device.GetComputeQueue(0).GetFamilyIndex());

			/* Create the result resource. */
			const uint16 chunkSize = static_cast<uint16>(RuntimeConfig::QueryInt(L"ChunkSize", 64));
			Image *img = new Image(device, ImageCreateInfo{ ImageType::Image2D, Format::R32_SFLOAT, Extent3D{chunkSize, chunkSize, 1}, 1, 1, SampleCountFlags::Pixel1Bit, ImageUsageFlags::Sampled | ImageUsageFlags::Storage });
			TextureStorage2D *texture = new TextureStorage2D(*img);

#ifdef _DEBUG
			if (chunkSize & (chunkSize - 1)) Log::Fatal("ChunkSize is not a power of 2!");
#endif

			/* Update the parent. */
			parent->lock.lock();
			parent->offsets.emplace_back(offset);
			parent->displacements.emplace_back(texture);
			parent->lock.unlock();

			/* Make sure that the chunk genertor is done loading. */
			while (!parent->perlinShader->IsLoaded()) PuThread::Pause();

			/* Initialize the descriptors. */
			descPool = new DescriptorPool(device, *parent->perlinShader, 1, 1);
			descSet = new DescriptorSet(*descPool, 0, parent->perlinShader->GetSetLayout(1));
			descSet->Write(parent->perlinShader->GetDescriptor("Result"), *texture);

			/* Record the commands. */
			cmdBuffer.Begin();
			cmdBuffer.AddLabel("Chunk Generation", Color::Red());
			cmdBuffer.BindComputePipeline(*parent->perlinPipe);
			cmdBuffer.BindGraphicsDescriptor(*parent->perlinPipe, *parent->perlinConstDescSet);
			cmdBuffer.BindGraphicsDescriptor(*parent->perlinPipe, *descSet);
			cmdBuffer.Dispatch(chunkSize >> 3, chunkSize >> 3, 1);
			cmdBuffer.End();

			device.GetComputeQueue(0).Submit(cmdBuffer);
			return Result::CustomWait();
		}

		Result Continue(void) final
		{
			delete descSet;
			delete descPool;
			return Result::AutoDelete();
		}

	protected:
		bool ShouldContinue(void) const final
		{
			return cmdBuffer.CanBegin();
		}

	private:
		SingleUseCommandBuffer cmdBuffer;
		DescriptorPool *descPool;
		DescriptorSet *descSet;
		ChunkGenerator *parent;
		Vector2 offset;
	};
}

Pu::ChunkGenerator::ChunkGenerator(AssetFetcher & fetcher, PhysicalWorld & world, uint32 maxChunks)
	: fetcher(&fetcher), world(&world), perlinPermutations(nullptr), perlinPipe(nullptr), 
	perlinDescPool(nullptr), perlinConstDescSet(nullptr), maxChunks(maxChunks)
{
	textures = &fetcher.FetchTexture2DArray("Terrain Textures", SamplerCreateInfo{}, true, 
		{
			L"{Textures}Terrain/Water.jpg",
			L"{Textures}Terrain/Grass.jpg",
			L"{Textures}Terrain/Dirt.jpg",
			L"{Textures}Terrain/Snow.jpg"
		});

	perlinShader = &fetcher.FetchComputepass(L"{Shaders}Perlin2D.comp.spv");
	perlinShader->PostLink.Add(*this, &ChunkGenerator::ComputePostLink);

	BufferLoadTask *task = new BufferLoadTask(fetcher.GetLoader(), mesh, perlinPermutations);
	TaskScheduler::Spawn(*task);
}

Pu::ChunkGenerator::ChunkGenerator(ChunkGenerator && value)
	: fetcher(value.fetcher), world(value.world), mesh(std::move(value.mesh)),
	textures(value.textures), perlinPermutations(value.perlinPermutations), 
	perlinShader(value.perlinShader), perlinPipe(value.perlinPipe), 
	perlinDescPool(value.perlinDescPool), maxChunks(value.maxChunks),
	perlinConstDescSet(value.perlinConstDescSet), offsets(std::move(value.offsets)), 
	displacements(std::move(value.displacements)), materials(std::move(value.materials))
{
	value.textures = nullptr;
	value.perlinPermutations = nullptr;
	value.perlinShader = nullptr;
	value.perlinPipe = nullptr;
	value.perlinDescPool = nullptr;
	value.perlinConstDescSet = nullptr;
}

Pu::ChunkGenerator & Pu::ChunkGenerator::operator=(ChunkGenerator && other)
{
	if (this != &other)
	{
		Destroy();

		fetcher = other.fetcher;
		world = other.world;
		mesh = std::move(other.mesh);
		textures = other.textures;
		perlinPermutations = other.perlinPermutations;
		perlinShader = other.perlinShader;
		perlinPipe = other.perlinPipe;
		perlinDescPool = other.perlinDescPool;
		perlinConstDescSet = other.perlinConstDescSet;
		maxChunks = other.maxChunks;
		offsets = std::move(other.offsets);
		displacements = std::move(other.displacements);
		materials = std::move(other.materials);
	}

	return *this;
}

void Pu::ChunkGenerator::Create(Vector2 offset)
{
#ifdef _DEBUG
	lock.lock();
	if (offsets.size() + 1 >= maxChunks) Log::Fatal("Chunk Generator cannot create a new chunk (limit reached)!");
	if (offsets.contains(offset)) Log::Fatal("Attempting to add chunk that has already been added!");
	lock.unlock();
#endif

	/* Allocate a new task if needed. */
	CreateChunkTask *task = new CreateChunkTask(this, offset);
	TaskScheduler::Spawn(*task);
}

void Pu::ChunkGenerator::Destroy(Vector2 offset)
{
	lock.lock();

	size_t i = 0;
	for (Vector2 cur : offsets)
	{
		if (offset == cur)
		{
			offsets.removeAt(i);

			delete displacements[i];
			displacements.removeAt(i);

			lock.unlock();
			return;
		}
	}

	lock.unlock();
	return;
}

void Pu::ChunkGenerator::Clear(void)
{
	lock.lock();

	for (TextureStorage2D *cur : displacements) delete cur;
	for (Terrain *cur : materials) delete cur;

	offsets.clear();
	displacements.clear();
	materials.clear();

	lock.unlock();
}

void Pu::ChunkGenerator::ComputePostLink(ShaderProgram &)
{
	perlinPipe = new ComputePipeline(*perlinShader, true);
	perlinDescPool = new DescriptorPool(fetcher->GetDevice(), *perlinShader);
	perlinDescPool->AddSet(0, 1);
	perlinDescPool->AddSet(1, maxChunks);
	perlinConstDescSet = new DescriptorSet(*perlinDescPool, 0, perlinShader->GetSetLayout(0));
}

void Pu::ChunkGenerator::Destroy(void)
{
	if (textures) fetcher->Release(*textures);
	if (perlinPermutations) delete perlinPermutations;
	if (perlinShader) fetcher->Release(*perlinShader);
	if (perlinPipe) delete perlinPipe;
	if (perlinConstDescSet) delete perlinConstDescSet;
	for (TextureStorage2D *texture : displacements) delete texture;
	for (Terrain *material : materials) delete material;
	if (perlinDescPool) delete perlinDescPool;
}