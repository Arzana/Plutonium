#pragma once
#include "Core/Events/EventBus.h"
#include "Graphics/Vulkan/Image.h"
#include "Graphics/Textures/ImageSaveFormats.h"

namespace Pu
{
	class CommandPool;

	/* Defines an object that can be used to save various assets to file. */
	class AssetSaver
	{
	public:
		/* Occurs when an image finishes it's loading procedures. */
		EventBus<const AssetSaver, const Image&> OnAssetSaved;

		/* Initializes a new instance of an asset saver. */
		AssetSaver(_In_ LogicalDevice &device);
		AssetSaver(_In_ const AssetSaver&) = delete;
		AssetSaver(_In_ AssetSaver&&) = delete;

		_Check_return_ AssetSaver& operator =(_In_ const AssetSaver&) = delete;
		_Check_return_ AssetSaver& operator =(_In_ AssetSaver&&) = delete;

		/* Saves the specified image to the specified location with the specified format. */
		void SaveImage(_In_ const Image &image, _In_ const wstring &path, _In_ ImageSaveFormats format);
		/* Saves the specified image data to the specified location with the specified format. */
		static void SaveImage(_In_ const void *data, _In_ uint32 width, _In_ uint32 height, _In_ Format format, _In_ const wstring &path, _In_ ImageSaveFormats saveFormat);

	private:
#ifdef _DEBUG
		friend class Font;
#endif

		LogicalDevice &device;
		Queue &graphicsQueue;

		static void SaveImageInternal(const void *data, int w, int h, int c, const wstring &path, ImageSaveFormats format);
	};
}