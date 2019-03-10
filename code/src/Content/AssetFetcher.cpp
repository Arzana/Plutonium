#include "Content/AssetFetcher.h"

Pu::AssetFetcher::AssetFetcher(TaskScheduler & scheduler, LogicalDevice & device)
{
	cache = new AssetCache();
	loader = new AssetLoader(scheduler, device, *cache);
}

Pu::AssetFetcher::~AssetFetcher(void)
{
	for (Texture *cur : textures)
	{
		Log::Warning("Deleting still referenced texture!");
		delete cur;
	}

	delete loader;
	delete cache;
}

Pu::Renderpass & Pu::AssetFetcher::FetchRenderpass(GraphicsPipeline & pipeline, std::initializer_list<wstring> subpasses)
{
	/* Create the hash and check if the cache contains the requested asset. */
	const size_t hash = std::hash<wstring>{}(subpasses);
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

Pu::Texture2D & Pu::AssetFetcher::FetchTexture2D(const wstring & path, const SamplerCreateInfo & samplerInfo, uint32 mipMapLevels)
{
	/* 
	The texture itself is not an asset but the sampler and image it stores are.
	So first get the sampler and then get the image.
	*/

	Sampler *sampler;

	/* Try to fetch the sampler, otherwise just create a new one. */
	size_t hash = Sampler::CreateHash(samplerInfo);
	if (cache->Contains(hash)) sampler = &cache->Get(hash).Duplicate<Sampler>(*cache);
	else
	{
		sampler = new Sampler(loader->GetDevice(), samplerInfo);
		sampler->loadedViaLoader = true;
		cache->Store(sampler);
	}

	/* Try to fetch the image, otherwise just create a new one. */
	hash = std::hash<wstring>{}(path);
	if (cache->Contains(hash))
	{
		Image &image = cache->Get(hash).Duplicate<Image>(*cache);

		/* Create the final texture and return it. */
		Texture2D *result = new Texture2D(image, *sampler);
		textures.push_back(result);
		return *result;
	}
	else
	{
		/* Create a new image and store it in cache, the hash is reset by us to the path for easier lookup. */
		const ImageInformation info = _CrtGetImageInfo(path);
		mipMapLevels = min(mipMapLevels, static_cast<uint32>(floor(log2(max(info.Width, info.Height))) + 1));
		const ImageCreateInfo createInfo(ImageType::Image2D, info.GetImageFormat(), Extent3D(info.Width, info.Height, 1), mipMapLevels, 1, SampleCountFlag::Pixel1Bit, ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled);
		Image *image = new Image(loader->GetDevice(), createInfo);
		image->SetHash(hash);
		cache->Store(image);

		/* Create the final texture and start the load/stage process. */
		Texture2D *result = new Texture2D(*image, *sampler);
		textures.push_back(result);
		loader->InitializeTexture(*result, path, info);
		return *result;
	}
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

void Pu::AssetFetcher::Release(Texture & texture)
{
	/* Make sure to release the underlying resources. */
	cache->Release(texture.Image);
	cache->Release(texture.Sampler);

	/* Delete the actual texture. */
	for (Texture *cur : textures)
	{
		if (&texture == cur)
		{
			textures.remove(cur);
			delete cur;
			return;
		}
	}
}