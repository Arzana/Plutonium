#include "Content/AssetLoader.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Graphics/Vulkan/Shaders/Shader.h"

Pu::AssetLoader::AssetLoader(TaskScheduler & scheduler, LogicalDevice & device, AssetCache & cache)
	: cache(cache), scheduler(scheduler), device(device), transferQueue(device.GetTransferQueue(0))
{
	cmdPool = new CommandPool(device, transferQueue.GetFamilyIndex());

	for (size_t i = 0; i < InitialLoadCommandBufferCount; i++)
	{
		AllocateCmdBuffer();
	}
}

Pu::AssetLoader::~AssetLoader(void)
{
	buffers.clear();
	delete cmdPool;
}

void Pu::AssetLoader::PopulateRenderpass(GraphicsPipeline & pipeline, Renderpass & renderpass, const vector<wstring> &subpasses)
{
	vector<std::tuple<size_t, wstring>> toLoad;

	for (const wstring &path : subpasses)
	{
		/* Create an unique indentifier for the subpass. */
		const size_t subpassHash = std::hash<wstring>{}(path);

		/* Check if the subpass is already loaded. */
		if (cache.Contains(subpassHash))
		{
			renderpass.shaders.emplace_back(cache.Get(subpassHash).Duplicate<Shader>(cache));
		}
		else
		{
			/* Create a new asset and store it in the cache. */
			Shader *shader = new Shader(device);
			cache.Store(shader);

			/* Add the subpass to the to-load list and add it to the renderpass. */
			toLoad.emplace_back(renderpass.shaders.size(), path);
			renderpass.shaders.emplace_back(*shader);
		}
	}

	/* The load task is deleted by the scheduler as the continue has an auto delete set. */
	GraphicsPipeline::LoadTask *loadTask = new GraphicsPipeline::LoadTask(pipeline, renderpass, toLoad);

	scheduler.Spawn(*loadTask);
}

void Pu::AssetLoader::FinalizeGraphicsPipeline(GraphicsPipeline & pipeline, Renderpass & renderpass)
{
	pipeline.renderpass = &renderpass;
	pipeline.Finalize();
}

void Pu::AssetLoader::InitializeTexture(Texture & texture, const wstring & path, const ImageInformation & info)
{
	class StageTask
		: public Task
	{
	public:
		StageTask(AssetLoader &parent, Texture &texture, const ImageInformation &info, const wstring &path)
			: result(texture), parent(parent), cmdBuffer(parent.GetCmdBuffer()), staged(false), name(path.fileNameWithoutExtension())
		{
			child = new Texture::LoadTask(texture, info, path);
			child->SetParent(*this);
		}

		virtual Result Execute(void) override
		{
			/* Just execute the texture load task. */
			scheduler->Spawn(*child);
			return Result::Default();
		}

		virtual Result Continue(void) override
		{
			/*
			There are two stages after the texels are loaded.
			First we stage the image data by applying a copy all command to the transfer queue.
			Seoncdly we recycle the buffer and mark the texture as loaded.
			*/
			if (staged)
			{
				parent.buffers.recycle(cmdBuffer);
				result.Image.MarkAsLoaded(true, std::move(name));
				delete child;
				return Result::AutoDelete();
			}
			else
			{
				/*  Begin the command buffer and add the memory barrier to ensure a good layout. */
				cmdBuffer.Begin();
				cmdBuffer.MemoryBarrier(result, PipelineStageFlag::TopOfPipe, PipelineStageFlag::Transfer, ImageLayout::TransferDstOptimal, AccessFlag::TransferWrite, result.GetFullRange());

				/* Copy actual data and end the buffer. */
				cmdBuffer.CopyEntireBuffer(child->GetStagingBuffer(), result.Image);
				cmdBuffer.End();

				parent.transferQueue.Submit(cmdBuffer);

				staged = true;
				return Result::CustomWait();
			}
		}

	protected:
		virtual bool ShouldContinue(void) const override
		{
			/* The texture is done staging if the buffer can begin again. */
			if (staged) return cmdBuffer.CanBegin();
			return Task::ShouldContinue();
		}

	private:
		Texture &result;
		AssetLoader &parent;
		CommandBuffer &cmdBuffer;
		Texture::LoadTask *child;
		wstring name;
		bool staged;
	};

	/* Simply create the task and spawn it. */
	StageTask *task = new StageTask(*this, texture, info, path);
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::InitializeFont(Font & font, const wstring & path, Task & continuation)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(AssetLoader &parent, Font &font, const wstring &path, Task &continuation)
			: result(font), parent(parent), cmdBuffer(parent.GetCmdBuffer()), 
			path(path), continuation(continuation)
		{}

		virtual Result Execute(void) override
		{
			/* Load the glyph information. */
			result.Load(path);
			const Vector2 imgSize = result.LoadGlyphInfo();

			/* Create and populate the staging buffer. */
			const size_t stagingBufferSize = static_cast<size_t>(imgSize.X) * static_cast<size_t>(imgSize.Y);
			buffer = new StagingBuffer(parent.device, stagingBufferSize);
			buffer->BeginMemoryTransfer();
			result.StageAtlas(imgSize, reinterpret_cast<byte*>(buffer->GetHostMemory()));
			buffer->EndMemoryTransfer();

			/* Create the result image. */
			const Extent3D extent(static_cast<uint32>(imgSize.X), static_cast<uint32>(imgSize.Y), 1);
			ImageCreateInfo info(ImageType::Image2D, Format::R8_UNORM, extent, 1, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled);
			result.atlasImg = new Image(parent.device, info);

			/* Make sure the atlas has the correct layout. */
			cmdBuffer.Begin();
			cmdBuffer.MemoryBarrier(*result.atlasImg, 
				PipelineStageFlag::TopOfPipe, 
				PipelineStageFlag::Transfer, 
				ImageLayout::TransferDstOptimal, 
				AccessFlag::TransferWrite, 
				result.atlasImg->GetFullRange(ImageAspectFlag::Color));

			/* Copy actual data and end the buffer. */
			cmdBuffer.CopyEntireBuffer(*buffer, *result.atlasImg);
			cmdBuffer.End();

			parent.transferQueue.Submit(cmdBuffer);
			return Result::CustomWait();
		}

		virtual Result Continue(void) override
		{
			/* Recycle the buffer as soon as possible. */
			parent.buffers.recycle(cmdBuffer);

			/* Mark both the image and the font as loaded, font will delete the atlas so just mark it as not loaded via the loader. */
			wstring name = L"Atlas ";
			name += path.fileNameWithoutExtension();
			result.atlasImg->MarkAsLoaded(false, std::move(name));

			/* Delete the staging buffer and this task. */
			delete buffer;
			return Result(&continuation, true, false);
		}

	protected:
		virtual bool ShouldContinue(void) const override
		{
			/* The texture is done staging if the buffer can begin again. */
			return cmdBuffer.CanBegin();
		}

	private:
		Font &result;
		AssetLoader &parent;
		Task &continuation;
		CommandBuffer &cmdBuffer;
		const wstring path;
		StagingBuffer *buffer;
	};

	/* Simply create the task and spawn it. */
	LoadTask *task = new LoadTask(*this, font, path, continuation);
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::AllocateCmdBuffer(void)
{
	buffers.emplace(std::move(cmdPool->Allocate()));
}

Pu::CommandBuffer& Pu::AssetLoader::GetCmdBuffer(void)
{
	/* Allocate a new command buffer if the queue is empty. */
	if (!buffers.available()) AllocateCmdBuffer();
	return buffers.get();
}