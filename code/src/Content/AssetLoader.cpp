#include "Content/AssetLoader.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Graphics/Vulkan/Shaders/Subpass.h"
#include "Config.h"

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
			renderpass.subpasses.emplace_back(static_cast<Subpass&>(cache.Get(subpassHash)));
		}
		else
		{
			/* Create a new asset and store it in the cache. */
			Subpass *subpass = new Subpass(device);
			cache.Store(subpass);

			/* Add the subpass to the to-load list and add it to the renderpass. */
			toLoad.emplace_back(renderpass.subpasses.size(), path);
			renderpass.subpasses.emplace_back(*subpass);
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
			: result(texture), parent(parent), cmdBuffer(parent.GetCmdBuffer()), staged(false)
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
				parent.RecycleCmdBuffer(cmdBuffer);
				result.Image.MarkAsLoaded(true);
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
		bool staged;
	};

	/* Simply create the task and spawn it. */
	StageTask *task = new StageTask(*this, texture, info, path);
	scheduler.Spawn(*task);
}

void Pu::AssetLoader::AllocateCmdBuffer(void)
{
	buffers.emplace_back(std::make_tuple(true, cmdPool->Allocate()));
}

Pu::CommandBuffer& Pu::AssetLoader::GetCmdBuffer(void)
{
	lock.lock();

	size_t i;
	for (i = 0; i < buffers.size(); i++)
	{
		/* Check if any buffer is usable. */
		if (std::get<0>(buffers[i]))
		{
			/* Set the buffer to used and return the command buffer with it's asset list. */
			std::get<0>(buffers[i]) = false;
			lock.unlock();
			return std::get<1>(buffers[i]);
		}
	}

	/* No viable command buffer was found so create a new one. */
	AllocateCmdBuffer();
	std::get<0>(buffers[i]) = false;

	lock.unlock();
	return std::get<1>(buffers[i]);
}

void Pu::AssetLoader::RecycleCmdBuffer(CommandBuffer & cmdBuffer)
{
	lock.lock();

	for (size_t i = 0; i < buffers.size(); i++)
	{
		/* Find the index of this command buffer and set it to usable again. */
		if (std::get<1>(buffers[i]).hndl == cmdBuffer.hndl)
		{
			std::get<0>(buffers[i]) = true;
			lock.unlock();
			return;
		}
	}

	/* this should never occur. */
	lock.unlock();
	Log::Warning("Cannot recycle command buffer (unable to find match)!");
	return;
}