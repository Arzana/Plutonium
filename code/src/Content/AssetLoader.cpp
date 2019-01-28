#include "Content/AssetLoader.h"

Pu::AssetLoader::AssetLoader(LogicalDevice & device, TaskScheduler & scheduler)
	: device(device), scheduler(scheduler)
{}

void Pu::AssetLoader::PopulateRenderpass(GraphicsPipeline & pipeline, Renderpass & result, const vector<string>& subpasses)
{
	vector<std::tuple<size_t, string>> toLoad;

	for (const string &path : subpasses)
	{
		/* Create unique indentifier for the subpass. */
		const size_t hash = std::hash<string>{}(path);

		/* Check if we've already loaded the subpass. */
		if (cache.Contains(hash))
		{
			/* Just add a new reference to the renderpass. */
			result.subpasses.emplace_back(&cache.Get<Subpass>(hash));
		}
		else
		{
			/* Create a new subpass and add it to the cache. */
			Subpass *subpass = new Subpass(device);
			cache.Add(subpass, hash);

			/* Add the subpass to the to load list and add it to the result. */
			toLoad.emplace_back(result.subpasses.size(), path);
			result.subpasses.emplace_back(subpass);
		}
	}

	/* The load task is deleted by the scheduler as the continue has an auto delete set. */
	GraphicsPipeline::LoadTask *loadTask = new GraphicsPipeline::LoadTask(pipeline, result, toLoad);
	scheduler.Spawn(*loadTask);
}

void Pu::AssetLoader::FinalizeGraphicsPipeline(GraphicsPipeline & pipeline, Renderpass & renderpass)
{
	pipeline.renderpass = &renderpass;
	pipeline.Finalize();
}