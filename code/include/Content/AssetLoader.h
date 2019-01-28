#pragma once
#include "IOCache.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	/* Defines an object used to load main assets onto a specific logical device. */
	class AssetLoader
	{
	public:
		/* Initializes a new instance of an asset loader for the specified logical device. */
		AssetLoader(_In_ LogicalDevice &device, _In_ TaskScheduler &scheduler);
		AssetLoader(_In_ const AssetLoader&) = delete;
		AssetLoader(_In_ AssetLoader&&) = delete;

		_Check_return_ AssetLoader& operator =(_In_ const AssetLoader&) = delete;
		_Check_return_ AssetLoader& operator =(_In_ AssetLoader&&) = delete;

		/* Gets the logical device associated with this loader. */
		_Check_return_ inline  LogicalDevice& GetDevice(void)
		{
			return device;
		}

		/* Gets the cache used by the loader. */
		_Check_return_ inline IoCache& GetCache(void)
		{
			return cache;
		}

		/* Starts the population of a render pass and loads the subpasses. */
		void PopulateRenderpass(_Out_ GraphicsPipeline &pipeline, _Out_ Renderpass &result, _In_ const vector<string> &subpasses);
		/* Finalizes a graphics pipeline with an existing renderpass. */
		void FinalizeGraphicsPipeline(_Out_ GraphicsPipeline &pipeline, _In_ Renderpass &renderpass);

	private:
		IoCache cache;
		TaskScheduler &scheduler;
		LogicalDevice &device;
	};
}