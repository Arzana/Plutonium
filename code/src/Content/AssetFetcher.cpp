#include "Content/AssetFetcher.h"

Pu::AssetFetcher::AssetFetcher(TaskScheduler & scheduler, LogicalDevice & device)
	: Patern()
{
	cache = new AssetCache();
	loader = new AssetLoader(scheduler, device, *cache);

	/* Add some of the default wildcards. */
	AddWildcard(L"{Assets}", L"../assets/");
	AddWildcard(L"{Shaders}", L"assets/shaders/");
	AddWildcard(L"{Textures}", L"{Assets}images/");
	AddWildcard(L"{Fonts}", L"{Assets}fonts/");
	AddWildcard(L"{Models}", L"assets/models/");
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

Pu::Renderpass & Pu::AssetFetcher::FetchRenderpass(std::initializer_list<std::initializer_list<wstring>> shaders)
{
	/* Make sure the wildcards are solved. */
	vector<vector<wstring>> mutableShaders;
	wstring hashParameter;
	for (const std::initializer_list<wstring> &subpass : shaders)
	{
		mutableShaders.emplace_back();

		for (const wstring &path : subpass)
		{
			mutableShaders.back().emplace_back(path);
			Solve(mutableShaders.back().back());
			hashParameter += mutableShaders.back().back();
		}
	}

	/* Create the hash and check if the cache contains the requested asset. */
	const size_t hash = std::hash<wstring>{}(hashParameter);
	if (cache->Contains(hash)) return cache->Get(hash).Duplicate<Renderpass>(*cache);

	/* Create a new renderpass and start loading it. */
	Renderpass *renderpass = new Renderpass(loader->GetDevice());
	renderpass->SetHash(hash);
	loader->PopulateRenderpass(*renderpass, mutableShaders);
	cache->Store(renderpass);
	return *renderpass;
}

Pu::Texture2D & Pu::AssetFetcher::FetchTexture2D(const PumTexture & texture)
{
	return FetchTexture2D(texture.Path.toWide(), texture.GetSamplerCreateInfo(), texture.IsSRGB);
}

Pu::Texture2D & Pu::AssetFetcher::FetchTexture2D(const wstring & path, const SamplerCreateInfo & samplerInfo, bool sRGB, uint32 mipMapLevels)
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
		textures.emplace_back(result);
		return *result;
	}
	else
	{
		/* Create a new image and store it in cache, the hash is reset by us to the path for easier lookup. */
		const ImageInformation info = _CrtGetImageInfo(mutablePath);
		mipMapLevels = min(mipMapLevels, Image::GetMaxMipLayers(info.Width, info.Height, 1));
		const ImageUsageFlag usage = ImageUsageFlag::TransferSrc | ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled;

		const ImageCreateInfo createInfo(ImageType::Image2D, info.GetImageFormat(sRGB), Extent3D(info.Width, info.Height, 1), mipMapLevels, 1, SampleCountFlag::Pixel1Bit, usage);
		Image *image = new Image(loader->GetDevice(), createInfo);
		image->SetDebugName(path.toUTF8().fileNameWithoutExtension());
		image->SetHash(hash);
		cache->Store(image);

		/* Create the final texture and start the load/stage process. */
		Texture2D *result = new Texture2D(*image, sampler);
		textures.emplace_back(result);
		loader->InitializeTexture(*result, mutablePath, info);
		return *result;
	}
}

Pu::TextureCube & Pu::AssetFetcher::FetchSkybox(const vector<wstring>& paths)
{
	return FetchTextureCube(L"Skybox", SamplerCreateInfo{}, true, paths, 1);
}

Pu::TextureCube & Pu::AssetFetcher::FetchTextureCube(const wstring & name, const SamplerCreateInfo & samplerInfo, bool sRGB, const vector<wstring>& paths, uint32 mipMapLevels)
{
	/* Make sure the wildcards are solved. */
	vector<wstring> mutableImages;
	wstring hashParameter;
	for (const wstring &path : paths)
	{
		mutableImages.emplace_back(path);
		Solve(mutableImages.back());
		hashParameter += mutableImages.back();
	}

	/*
	The texture itself is not an asset but the sampler and image it stores are.
	So first get the sampler and then get the image.
	*/
	Sampler &sampler = FetchSampler(samplerInfo);

	/* Create the hash and check if the cache contains the requested asset. */
	const size_t hash = std::hash<wstring>{}(hashParameter);
	if (cache->Contains(hash))
	{
		Image &image = cache->Get(hash).Duplicate<Image>(*cache);

		/* Create the final texture and return it. */
		TextureCube *result = new TextureCube(image, sampler);
		textures.emplace_back(result);
		return *result;
	}
	else
	{
		/* Create a new image and store it in cache, the hash is reset by us to the path for easier lookup. */
		const ImageInformation info = _CrtGetImageInfo(mutableImages.front());
		mipMapLevels = min(mipMapLevels, Image::GetMaxMipLayers(info.Width, info.Height, 1));
		const ImageUsageFlag usage = ImageUsageFlag::TransferSrc | ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled;

		ImageCreateInfo createInfo(ImageType::Image2D, info.GetImageFormat(sRGB), Extent3D(info.Width, info.Height, 1), mipMapLevels, 6, SampleCountFlag::Pixel1Bit, usage);
		createInfo.Flags = ImageCreateFlag::CubeCompatible;
		Image *image = new Image(loader->GetDevice(), createInfo);
		image->SetDebugName(name.toUTF8());
		image->SetHash(hash);
		cache->Store(image);

		/* Create the final texture and start the load/stage process. */
		TextureCube *result = new TextureCube(*image, sampler);
		textures.emplace_back(result);
		loader->InitializeTexture(*result, mutableImages, info, name);
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
		CreateTextureTask(Font &result, AssetFetcher &parent, const wstring &path)
			: result(result), parent(parent), path(path)
		{}

		virtual Result Execute(void) override
		{
			SamplerCreateInfo info(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::ClampToBorder);
			result.atlasTex = new Texture2D(*result.atlasImg, parent.FetchSampler(info));
			result.MarkAsLoaded(true, path.fileNameWithoutExtension());
			return Result::AutoDelete();
		}

	private:
		const wstring path;
		Font &result;
		AssetFetcher &parent;
	};

	/* Create the font and add it to the cache. */
	Font *result = new Font(size, codeChart);
	result->SetHash(hash);
	cache->Store(result);

	/* Load the font. */
	CreateTextureTask *continuation = new CreateTextureTask(*result, *this, mutablePath);
	loader->InitializeFont(*result, mutablePath, *continuation);
	return *result;
}

Pu::Model & Pu::AssetFetcher::FetchModel(const wstring & path, const DeferredRenderer & deferredRenderer, const LightProbeRenderer & probeRenderer)
{
	/* Solve for the model path. */
	wstring mutablePath(path);
	Solve(mutablePath);

	/* Try to fetch the model, create a new one. */
	const size_t hash = std::hash<wstring>{}(mutablePath);
	if (cache->Contains(hash)) return cache->Get(hash).Duplicate<Model>(*cache);

	/* Create a new model and fetch its textures. */
	Model *result = new Model();
	PuMData data = PuMData::TexturesOnly(mutablePath);
	result->textures.reserve(data.Textures.size());
	for (const PumTexture &texture : data.Textures) result->textures.emplace_back(&FetchTexture2D(texture));

	/* Add the default textures to the models list. */
	result->textures.emplace_back(&GetDefaultDiffuse());
	result->textures.emplace_back(&GetDefaultSpecGloss());
	result->textures.emplace_back(&GetDefaultNormal());

	/* Start the parallel initialization and return the model reference. */
	loader->InitializeModel(*result, mutablePath, deferredRenderer, probeRenderer);
	cache->Store(result);
	return *result;
}

Pu::Texture2D & Pu::AssetFetcher::CreateTexture2D(const string & id, Color color)
{
	return CreateTexture2D(id, color.ToArray(), 1, 1, Format::R8G8B8A8_SRGB, SamplerCreateInfo());
}

Pu::Texture2D& Pu::AssetFetcher::CreateTexture2D(const string & id, const void * data, uint32 width, uint32 height, Format format, const SamplerCreateInfo & samplerInfo)
{
	/*
	The texture itself is not an asset but the sampler and image it stores are.
	So first get the sampler and then get the image.
	*/
	Sampler &sampler = FetchSampler(samplerInfo);

	/* Try to fetch the image, otherwise just create a new one. */
	const size_t hash = std::hash<string>{}(id);
	if (cache->Contains(hash))
	{
		Image &image = cache->Get(hash).Duplicate<Image>(*cache);

		/* Create the final texture and return it. */
		Texture2D *result = new Texture2D(image, sampler);
		textures.emplace_back(result);
		return *result;
	}
	else
	{
		/* Create a new image and store it in cache, the hash is reset by us to the path for easier lookup. */
		const uint32 mipMapLevels = Image::GetMaxMipLayers(width, height, 1);
		const ImageUsageFlag usage = ImageUsageFlag::TransferSrc | ImageUsageFlag::TransferDst | ImageUsageFlag::Sampled;

		const ImageCreateInfo createInfo(ImageType::Image2D, format, Extent3D(width, height, 1), mipMapLevels, 1, SampleCountFlag::Pixel1Bit, usage);
		Image *image = new Image(loader->GetDevice(), createInfo);
		image->SetDebugName(id);
		image->SetHash(hash);
		cache->Store(image);

		/* Create the final texture and start the load/stage process. */
		Texture2D *result = new Texture2D(*image, sampler);
		textures.emplace_back(result);
		loader->InitializeTexture(*result, reinterpret_cast<const byte*>(data), width * height * 4, std::move(id.toWide()));
		return *result;
	}
}

void Pu::AssetFetcher::Release(Renderpass & renderpass)
{
	/* Make sure to release the subpasses as well. */
	for (const Subpass &subpass : renderpass.subpasses)
	{
		for (Shader *shader : subpass.GetShaders())
		{
			cache->Release(*shader);
		}
	}

	cache->Release(renderpass);
}

void Pu::AssetFetcher::Release(Texture & texture)
{
	/* Make sure to release the underlying resources. */
	cache->Release(*texture.Image);
	Release(*texture.Sampler);

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
	Release(*font.atlasTex->Sampler);
	cache->Release(font);
}

void Pu::AssetFetcher::Release(Model & model)
{
	/* Relaese the underlying textures. */
	for (Texture2D *cur : model.textures) Release(*cur);
	cache->Release(model);
}

Pu::Texture2D & Pu::AssetFetcher::GetDefaultDiffuse(void)
{
	return CreateTexture2D("Default_Diffuse", Color::White());
}

Pu::Texture2D & Pu::AssetFetcher::GetDefaultSpecGloss(void)
{
	return CreateTexture2D("Default_SpecularGlossiness", Color::Black());
}

Pu::Texture2D & Pu::AssetFetcher::GetDefaultNormal(void)
{
	return CreateTexture2D("Default_Normal", Color::Malibu());
}