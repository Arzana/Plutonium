#pragma once
#include "AssetLoader.h"
#include "Graphics/Textures/Texture2D.h"

namespace Pu
{
	/* Defines an object used to fetch assets from either cache or from file. */
	class AssetFetcher
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
		_Check_return_ Renderpass& FetchRenderpass(_In_ GraphicsPipeline &pipeline, _In_ std::initializer_list<string> subpasses);
		/* Fetches the desired 2D texture. */
		_Check_return_ Texture2D& FetchTexture2D(_In_ const string &path, _In_ const SamplerCreateInfo &samplerInfo, _In_opt_ uint32 mipMapLevels = DefaultMipLevels);

		/* Releases the renderpass. */
		void Release(_In_ GraphicsPipeline &pipeline);
		/* Releases the texture. */
		void Release(_In_ Texture &texture);

	private:
		AssetLoader *loader;
		AssetCache *cache;

		vector<Texture*> textures;
	};
}