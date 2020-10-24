#pragma once
#include "Patern.h"
#include "PumLoader.h"
#include "AssetLoader.h"
#include "Graphics/Textures/TextureCube.h" 

namespace Pu
{
	/* Defines an object used to fetch assets from either cache or from file. */
	class AssetFetcher
		: public Patern
	{
	public:
		/* Initializes a new instance of an asset fetcher. */
		AssetFetcher(_In_ LogicalDevice &device);
		AssetFetcher(_In_ const AssetFetcher&) = delete;
		AssetFetcher(_In_ AssetFetcher&&) = delete;
		/* Releases the resources allocated by the asset fetcher. */
		~AssetFetcher(void);

		_Check_return_ AssetFetcher& operator =(_In_ const AssetFetcher&) = delete;
		_Check_return_ AssetFetcher& operator =(_In_ AssetFetcher&&) = delete;

		/* Fetches the desired renderpass or replaces an already existing one. */
		_Check_return_ Renderpass& FetchRenderpass(_In_ Renderpass *old, _In_ const vector<vector<wstring>> &shaders);
		/* Fetches the desired compute program. */
		_Check_return_ ShaderProgram& FetchComputepass(_In_ const wstring &shader);
		/* Fetches the desired 2D texture with the correct sampler and color space. */
		_Check_return_ Texture2D& FetchTexture2D(_In_ const PumTexture &texture);
		/* Fetches the desired 2D texture. */
		_Check_return_ Texture2D& FetchTexture2D(_In_ const wstring &path, _In_ const SamplerCreateInfo &samplerInfo, _In_ bool sRGB, _In_opt_ uint32 mipMapLevels = DefaultMipLevels);
		/* Fetches the destired cube texture as a skybox. */
		_Check_return_ TextureCube& FetchSkybox(_In_ const vector<wstring> &paths);
		/* Fetches the desired cube texture. Texture order: right, left, top, bottom, front, back. */
		_Check_return_ TextureCube& FetchTextureCube(_In_ const string &name, _In_ const SamplerCreateInfo &samplerInfo, _In_ bool sRGB, _In_ const vector<wstring> &paths, _In_opt_ uint32 mipMapLevels = DefaultMipLevels);
		/* Fetches the desired textures togething into a 2D array texture. */
		_Check_return_ Texture2DArray& FetchTexture2DArray(_In_ const string &name, _In_ const SamplerCreateInfo &samplerInfo, _In_ bool sRGB, _In_ const vector<wstring> &paths, _In_opt_ uint32 mipMapLevels = DefaultMipLevels);
		/* Fetches the desired sampler. */
		_Check_return_ Sampler& FetchSampler(_In_ const SamplerCreateInfo &samplerInfo);
		/* Fetches the desired font. */
		_Check_return_ Font& FetchFont(_In_ const wstring &path, _In_ float size, _In_ const CodeChart &codeChart);
		/* Fetches the desired model. */
		_Check_return_ Model& FetchModel(_In_ const wstring &path, _In_ const DeferredRenderer &deferredRenderer, _In_ const LightProbeRenderer *probeRenderer);

		/* Creates a default 2D texture (1x1) from a specified color. */
		_Check_return_ Texture2D& CreateTexture2D(_In_ const string &id, _In_ Color color);
		/* Creates a new 2D texture from the specified RGBA data. */
		_Check_return_ Texture2D& CreateTexture2D(_In_ const string &id, _In_ const void *data, _In_ uint32 width, _In_ uint32 height, _In_ Format format, _In_ const SamplerCreateInfo &samplerInfo);
		/* Creates a primitive model with a diffuse texture. */
		_Check_return_ Model& CreateModel(_In_ ShapeType type, _In_ const DeferredRenderer &deferredRenderer, _In_opt_ const LightProbeRenderer *probeRenderer, _In_ const wstring &diffuse);
		/* Creates a primitive model with optional textures. */
		_Check_return_ Model& CreateModel(_In_ ShapeType type, _In_ const DeferredRenderer &deferredRenderer, _In_opt_ const LightProbeRenderer *probeRenderer = nullptr, _In_opt_ Texture2D *diffuse = nullptr, _In_opt_ Texture2D *specularGloss = nullptr);

		/* Releases the renderpass. */
		void Release(_In_ Renderpass &renderpass);
		/* Releases the shader program. */
		void Release(_In_ ShaderProgram &program);
		/* Releases the texture. */
		void Release(_In_ Texture &texture);
		/* Releases the sampler. */
		void Release(_In_ Sampler &sampler);
		/* Releases the font. */
		void Release(_In_ Font &font);
		/* Releases the model. */
		void Release(_In_ Model &model);

		/* Gets the logical device that this asset fetcher uses. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return loader->GetDevice();
		}

		/* Gets the underlying asset loader. */
		_Check_return_ inline AssetLoader& GetLoader(void)
		{
			return *loader;
		}

		/* Visualizes the assets currently active through this loader. */
		inline void Visualize(void) const
		{
			cache->Visualize();
		}

	private:
		AssetLoader *loader;
		AssetCache *cache;

		vector<Texture*> textures;

		Sampler& FetchSamplerForImage(_In_ const ImageInformation &info, const SamplerCreateInfo &createInfo, bool anisotropy);
		Texture2DArray& FetchMultiTexture(const string &name, const SamplerCreateInfo &samplerInfo, bool sRGB, const vector<wstring> &paths, uint32 mipMapLevels, ImageViewType view);
		Texture2D& GetDefaultDiffuse(void);
		Texture2D& GetDefaultSpecGloss(void);
	};
}