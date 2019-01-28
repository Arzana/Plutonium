#pragma once
#include "AssetLibrary.h"
#include "AssetLoader.h"

namespace Pu
{
	/* Defines an object for fetching assets fast. */
	class AssetFetcher
	{
	public:
		/* Initializes a new instance of an asset fetcher. */
		AssetFetcher(_In_ LogicalDevice &device, _In_ TaskScheduler &scheduler);
		AssetFetcher(_In_ const AssetFetcher&) = delete;
		AssetFetcher(_In_ AssetFetcher&&) = delete;

		_Check_return_ AssetFetcher& operator =(_In_ const AssetFetcher&) = delete;
		_Check_return_ AssetFetcher& operator =(_In_ AssetFetcher&&) = delete;

		/* Fetches the specified renderpass and attaches it to the graphics pipeline. */
		_Check_return_ Renderpass& FetchRenderpass(_Out_ GraphicsPipeline &pipeline, _In_ const vector<string> &subpasses);
		/* Frees the specific renderpass. */
		void FreeRenderpass(_In_ GraphicsPipeline &pipeline);

	private:
		AssetLibrary lib;
		AssetLoader loader;
	};
}