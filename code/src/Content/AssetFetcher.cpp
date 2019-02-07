#include "Content/AssetFetcher.h"

Pu::AssetFetcher::AssetFetcher(TaskScheduler & scheduler, LogicalDevice & device)
{
	cache = new AssetCache();
	loader = new AssetLoader(scheduler, device, *cache);
}

Pu::AssetFetcher::~AssetFetcher(void)
{
	delete loader;
	delete cache;
}

Pu::Renderpass & Pu::AssetFetcher::FetchRenderpass(GraphicsPipeline & pipeline, std::initializer_list<string> subpasses)
{
	/* Create the hash and check if the cache contians the requested asset. */
	const size_t hash = std::hash<string>{}(subpasses);
	if (cache->Contains(hash))
	{
		/* Duplicate the top level asset and finalize the graphics pipeline with the duplicated renderpass. */
		Renderpass &renderpass = cache->Get(hash).Duplicate<Renderpass>(*cache);
		loader->FinalizeGraphicsPipeline(pipeline, renderpass);
		return renderpass;
	}

	/* Create a new renderpass and start loading it. */
	Renderpass *renderpass = new Renderpass(loader->GetDevice());
	loader->PopulateRenderpass(pipeline, *renderpass, subpasses);
	cache->Store(renderpass);
	return *renderpass;
}

void Pu::AssetFetcher::Release(GraphicsPipeline & pipeline)
{
	/* Make sure to release the subpasses as well. */
	for (Subpass &cur : pipeline.GetRenderpass().subpasses)
	{
		cache->Release(cur);
	}

	/* const cast here shouldn't matter too much as this is basically a delete statement. */
	cache->Release(const_cast<Renderpass&>(pipeline.GetRenderpass()));
}