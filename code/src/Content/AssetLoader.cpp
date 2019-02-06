#include "Content/AssetLoader.h"
#include "Graphics/Vulkan/CommandPool.h"
#include "Graphics/Vulkan/Shaders/Subpass.h"
#include "Config.h"

Pu::AssetLoader::AssetLoader(TaskScheduler & scheduler, LogicalDevice & device, AssetCache & cache)
	: cache(cache), scheduler(scheduler), device(device), transferQueue(device.GetTransferQueue(0))
{
	cmdPool = new CommandPool(device, transferQueue.GetFamilyIndex());
	submitSemaphore = new Semaphore(device);

	for (size_t i = 0; i < InitialLoadCommandBufferCount; i++)
	{
		AllocateCmdBuffer();
	}
}

Pu::AssetLoader::~AssetLoader(void)
{
	buffers.clear();

	delete submitSemaphore;
	delete cmdPool;
}

void Pu::AssetLoader::PopulateRenderpass(GraphicsPipeline & pipeline, Renderpass & renderpass, std::initializer_list<string> subpasses)
{
	vector<std::tuple<size_t, string>> toLoad;

	for (const string &path : subpasses)
	{
		/* Create an unique indentifier for the subpass. */
		const size_t subpassHash = std::hash<string>{}(path);

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

void Pu::AssetLoader::AllocateCmdBuffer(void)
{
	buffers.emplace_back(std::make_tuple(true, cmdPool->Allocate(), vector<std::reference_wrapper<Asset>>()));
}

Pu::AssetLoader::buffer_t_ref Pu::AssetLoader::GetCmdBuffer(void)
{
	size_t i;
	for (i = 0; i < buffers.size(); i++)
	{
		/* Check if any buffer is usable. */
		if (std::get<0>(buffers[i]))
		{
			/* Set the buffer to used and return the command buffer with it's asset list. */
			std::get<0>(buffers[i]) = false;
			return buffer_t_ref(std::get<1>(buffers[i]), std::get<2>(buffers[i]));
		}
	}

	/* No viable command buffer was found so create a new one. */
	AllocateCmdBuffer();
	return buffer_t_ref(std::get<1>(buffers[i]), std::get<2>(buffers[i]));
}