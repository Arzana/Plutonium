#pragma once
#include "Graphics/Vulkan/Image.h"
#include "Core/Collections/pool.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Textures/ImageSaveFormats.h"

namespace Pu
{
	class CommandPool;

	/* Defines an object that can be used to save various assets to file. */
	class AssetSaver
	{
	public:
		/* Initializes a new instance of an asset saver. */
		AssetSaver(_In_ TaskScheduler &scheduler, _In_ LogicalDevice &device);
		AssetSaver(_In_ const AssetSaver&) = delete;
		AssetSaver(_In_ AssetSaver&&) = delete;
		/* Releases the resources allocated by the asset saver. */
		~AssetSaver(void);

		_Check_return_ AssetSaver& operator =(_In_ const AssetSaver&) = delete;
		_Check_return_ AssetSaver& operator =(_In_ AssetSaver&&) = delete;

		/* Saves the specified image to the specified location with the specified format. */
		void SaveImage(_In_ const Image &image, _In_ const wstring &path, _In_ ImageSaveFormats format);

	private:
		TaskScheduler &scheduler;
		LogicalDevice &device;
		CommandPool *cmdPool;

		Queue &transferQueue;
		pool<CommandBuffer> buffers;

		void AllocateCmdBuffer(void);
		CommandBuffer& GetCmdBuffer(void);
	};
}