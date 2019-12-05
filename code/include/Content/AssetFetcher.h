#pragma once
#include "Patern.h"
#include "PumLoader.h"
#include "AssetLoader.h"
#include "Graphics/Textures/Texture2D.h"

namespace Pu
{
	/* Defines an object used to fetch assets from either cache or from file. */
	class AssetFetcher
		: public Patern
	{
	public:
		/* Initializes a new instance of an asset fetcher. */
		AssetFetcher(_In_ TaskScheduler &scheduler, _In_ LogicalDevice &device);
		AssetFetcher(_In_ const AssetFetcher&) = delete;
		AssetFetcher(_In_ AssetFetcher&&) = delete;
		/* Releases the resources allocated by the asset fetcher. */
		~AssetFetcher(void);

		_Check_return_ AssetFetcher& operator =(_In_ const AssetFetcher&) = delete;
		_Check_return_ AssetFetcher& operator =(_In_ AssetFetcher&&) = delete;

		/* Fetches the desired renderpass. */
		_Check_return_ Renderpass& FetchRenderpass(_In_ std::initializer_list<std::initializer_list<wstring>> shaders);
		/* Fetches the desired 2D texture with the correct sampler and color space. */
		_Check_return_ Texture2D& FetchTexture2D(_In_ const PumTexture &texture);
		/* Fetches the desired 2D texture. */
		_Check_return_ Texture2D& FetchTexture2D(_In_ const wstring &path, _In_ const SamplerCreateInfo &samplerInfo, _In_opt_ bool sRGB, _In_opt_ uint32 mipMapLevels = DefaultMipLevels);
		/* Fetches the desired sampler. */
		_Check_return_ Sampler& FetchSampler(_In_ const SamplerCreateInfo &samplerInfo);
		/* Fetches the desired font. */
		_Check_return_ Font& FetchFont(_In_ const wstring &path, _In_ float size, _In_ const CodeChart &codeChart);

		/* Creates a new 2D texture from the specified RGBA data. */
		_Check_return_ Texture2D& CreateTexture2D(_In_ const string &id, _In_ const byte *data, _In_ uint32 width, _In_ uint32 height, _In_ const SamplerCreateInfo &samplerInfo);

		/* Releases the renderpass. */
		void Release(_In_ Renderpass &renderpass);
		/* Releases the texture. */
		void Release(_In_ Texture &texture);
		/* Releases the sampler. */
		void Release(_In_ Sampler &sampler);
		/* Releases the font. */
		void Release(_In_ Font &font);

	private:
		AssetLoader *loader;
		AssetCache *cache;

		vector<Texture*> textures;
	};
}