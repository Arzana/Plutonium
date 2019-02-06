#pragma once
#include "AssetLoader.h"

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

		/* Releases the renderpass. */
		void Release(_In_ GraphicsPipeline &pipeline);

	private:
		AssetLoader *loader;
		AssetCache *cache;
	};
}