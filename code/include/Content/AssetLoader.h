#pragma once
#include "AssetCache.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

namespace Pu
{
	class CommandPool;

	/* Defines an object that can be used to load various assets from file. */
	class AssetLoader
	{
	public:
		/* Initializes a new instance of an asset loader. */
		AssetLoader(_In_ TaskScheduler &scheduler, _In_ LogicalDevice &device, _In_ AssetCache &cache);
		AssetLoader(_In_ const AssetLoader&) = delete;
		AssetLoader(_In_ AssetLoader&&) = delete;
		/* Releases the resources allocated by the asset loader. */
		~AssetLoader(void);

		_Check_return_ AssetLoader& operator =(_In_ const AssetLoader&) = delete;
		_Check_return_ AssetLoader& operator =(_In_ AssetLoader&&) = delete;

		/* Gets the logical device associated with the loader. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return device;
		}

		/* Loads the renderpass with the specified subpasses and finalizes the graphics pipeline. */
		void PopulateRenderpass(_In_ GraphicsPipeline &pipeline, Renderpass &renderpass, _In_ std::initializer_list<string> subpasses);
		/* Finalizes the graphics pipeline. */
		void FinalizeGraphicsPipeline(_In_ GraphicsPipeline &pipeline, Renderpass &renderpass);

	private:
		using asset_t = std::reference_wrapper<Asset>;
		using buffer_t = std::tuple<bool, CommandBuffer, vector<asset_t>>;
		using buffer_t_ref = std::tuple<CommandBuffer&, vector<asset_t>&>;

		AssetCache &cache;
		TaskScheduler &scheduler;
		LogicalDevice &device;
		CommandPool *cmdPool;

		Queue &transferQueue;
		Semaphore *submitSemaphore;
		vector<buffer_t> buffers;
		std::mutex lock;

		void AllocateCmdBuffer(void);
		buffer_t_ref GetCmdBuffer(void);
	};
}