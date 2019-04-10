#include "Content/AssetFetcher.h"

Pu::AssetFetcher::AssetFetcher(TaskScheduler & scheduler, LogicalDevice & device)
	: Patern()
{
	cache = new AssetCache();
	loader = new AssetLoader(scheduler, device, *cache);

	/* Add some of the default wildcards. */
	AddWildcard(L"{Assets}", L"../assets/");
	AddWildcard(L"{Shaders}", L"{Assets}shaders/");
	AddWildcard(L"{Textures}", L"{Assets}images/");
	AddWildcard(L"{Fonts}", L"{Assets}fonts/");
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
	/* Make sure the wildcards are solved. */
	const vector<wstring> mutableSubpasses = Solve(subpasses);

	/* Create the hash and check if the cache contains the requested asset. */
	const size_t hash = std::hash<wstring>{}(mutableSubpasses);
	if (cache->Contains(hash))
	{
		/* Duplicate the top level asset and finalize the graphics pipeline with the duplicated renderpass. */
		Renderpass &renderpass = cache->Get(hash).Duplicate<Renderpass>(*cache);
		loader->FinalizeGraphicsPipeline(pipeline, renderpass);
		return renderpass;
	}

	/* Create a new renderpass and start loading it. */
	Renderpass *renderpass = new Renderpass(loader->GetDevice());
	loader->PopulateRenderpass(pipeline, *renderpass, mutableSubpasses);
	cache->Store(renderpass);
	return *renderpass;
}

Pu::Texture2D & Pu::AssetFetcher::FetchTexture2D(const wstring & path, const SamplerCreateInfo & samplerInfo, uint32 mipMapLevels)
{
	/* Solve for the texture path. */
	wstring mutablePath(path);
	Solve(mutablePath);

	/* 
	The texture itself is not an asset but the sampler and image it stores are.
	So first get the sampler and then get the image.
	*/
	Sampler &sampler = FetchSampler(samplerInfo);

	/* Try to fetch the image, otherwise just create a new one. */
	const size_t hash = std::hash<wstring>{}(mutablePath);
	if (cache->Contains(hash))
	{
		Image &image = cache->Get(hash).Duplicate<Image>(*cache);

		/* Create the final texture and return it. */
		Texture2D *result = new Texture2D(image, sampler);
		textures.push_back(result);
		return *result;
	}
	else
	{
		/* Create a new image and store it in cache, the hash is reset by us to the path for easier lookup. */
		const ImageInformation info = _CrtGetImageInfo(mutablePath);
		mipMapLevels = min(mipMapLevels, static_cast<uint32>(floor(log2(max(info.Width, info.Height))) + 1));
		ImageUsageFlag usage = ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled;
		if (AllowSaveOnLoadedImages) usage |= ImageUsageFlag::TransferSrc;

		const ImageCreateInfo createInfo(ImageType::Image2D, info.GetImageFormat(), Extent3D(info.Width, info.Height, 1), mipMapLevels, 1, SampleCountFlag::Pixel1Bit, usage);
		Image *image = new Image(loader->GetDevice(), createInfo);
		image->SetHash(hash);
		cache->Store(image);

		/* Create the final texture and start the load/stage process. */
		Texture2D *result = new Texture2D(*image, sampler);
		textures.push_back(result);
		loader->InitializeTexture(*result, mutablePath, info);
		return *result;
	}
}

Pu::Sampler & Pu::AssetFetcher::FetchSampler(const SamplerCreateInfo & samplerInfo)
{
	/* Try to fetch the sampler, otherwise just create a new one. */
	size_t hash = Sampler::CreateHash(samplerInfo);
	if (cache->Contains(hash)) return cache->Get(hash).Duplicate<Sampler>(*cache);
	else
	{
		Sampler *result = new Sampler(loader->GetDevice(), samplerInfo);
		result->loadedViaLoader = true;
		cache->Store(result);
		return *result;
	}
}

Pu::Font & Pu::AssetFetcher::FetchFont(const wstring & path, float size, const CodeChart & codeChart)
{
	/* Solve for the font path. */
	wstring mutablePath(path);
	Solve(mutablePath);

	/* Try to fetch the font, create a new one. */
	const size_t hash = std::hash<wstring>{}(mutablePath);
	if (cache->Contains(hash)) return cache->Get(hash).Duplicate<Font>(*cache);

	/* The sampler needs to be retrieved from the fetcher so we just add this as a continuation task. */
	class CreateTextureTask
		: public Task
	{
	public:
		CreateTextureTask(Font &result, AssetFetcher &parent)
			: result(result), parent(parent)
		{}

		virtual Result Execute(void) override
		{
			SamplerCreateInfo info(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToBorder);
			result.atlasTex = new Texture2D(*result.atlasImg, parent.FetchSampler(info));
			return Result::AutoDelete();
		}

	private:
		Font &result;
		AssetFetcher &parent;
	};

	/* Create the font and add it to the cache. */
	Font *result = new Font(loader->GetDevice(), size, codeChart);
	result->SetHash(hash);
	cache->Store(result);

	/* Load the font. */
	CreateTextureTask *continuation = new CreateTextureTask(*result, *this);
	loader->InitializeFont(*result, mutablePath, *continuation);
	return *result;
}

void Pu::AssetFetcher::Release(GraphicsPipeline & pipeline)
{
	/* Make sure to release the subpasses as well. */
	for (Shader &cur : pipeline.GetRenderpass().shaders)
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
	Release(texture.Sampler);

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

void Pu::AssetFetcher::Release(Sampler & sampler)
{
	cache->Release(sampler);
}

void Pu::AssetFetcher::Release(Font & font)
{
	Release(font.atlasTex->Sampler);
	cache->Release(font);
}