#include "Content/AssetFetcher.h"

Pu::AssetFetcher::AssetFetcher(LogicalDevice & device, TaskScheduler & scheduler)
	: loader(device, scheduler)
{}

Pu::Renderpass & Pu::AssetFetcher::FetchRenderpass(GraphicsPipeline & pipeline, const vector<string>& subpasses)
{
	/* Safety check. */
	if (pipeline.IsLoaded()) Log::Fatal("Cannot fetch renderpass for already loaded graphics pipeline!");

	/* Get an instance of the renderpass. */
	const size_t hash = std::hash<string>{}(subpasses);
	auto[found, asset] = lib.GetOrNew<Renderpass, DuplicationType::Reference, LogicalDevice&>(hash, loader.GetDevice());

	/* Populate the renderpass if it hasn't been loaded yet, otherwise; finalize the pipeline. */
	if (!found) loader.PopulateRenderpass(pipeline, asset, subpasses);
	else loader.FinalizeGraphicsPipeline(pipeline, asset);

	return asset;
}

void Pu::AssetFetcher::FreeRenderpass(GraphicsPipeline & pipeline)
{
	/* Early out of the pipeline doesn't have a renderpass. */
	if (!pipeline.IsLoaded()) return;

	/* Get subpass names for the hash. */
	vector<string> subpasses;
	for (const Subpass *cur : pipeline.GetRenderpass().subpasses)
	{
		subpasses.emplace_back(cur->GetName());
	}

	/* Recreate hash and try to free the asset. */
	lib.Free<Renderpass>(std::hash<string>{}(subpasses));

	/* Try to free all subpasses. */
	for (const string &cur : subpasses)
	{
		loader.GetCache().Free<Subpass>(std::hash<string>{}(cur));
	}
}