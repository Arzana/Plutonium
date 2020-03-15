#pragma once
#include "AssetCache.h"
#include "Core/Threading/Tasks/Scheduler.h"
#include "Graphics/Vulkan/Pipelines/GraphicsPipeline.h"
#include "Graphics/Text/Font.h"
#include "Graphics/Models/Model.h"

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

		_Check_return_ AssetLoader& operator =(_In_ const AssetLoader&) = delete;
		_Check_return_ AssetLoader& operator =(_In_ AssetLoader&&) = delete;

		/* Gets the logical device associated with the loader. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return device;
		}

		/* Loads the renderpass with the specified subpasses and finalizes the graphics pipeline. */
		void PopulateRenderpass(_In_ Renderpass &renderpass, _In_ const vector<vector<wstring>> &shaders);
		/* Loads and stages a texture from a specific path. */
		void InitializeTexture(_In_ Texture &texture, _In_ const wstring &path, _In_ const ImageInformation &info);
		/* Loads and stages a texture from 6 individual cube map image paths. */
		void InitializeTexture(_In_ Texture &texture, _In_ const vector<wstring> &paths, _In_ const ImageInformation &info, _In_ const wstring &name);
		/* Stages a texture from the specified source. */
		void InitializeTexture(_In_ Texture &texture, _In_ const byte *data, _In_ size_t size, _In_ wstring &&id);
		/* Generates mipmaps for the texture and makes it shader read only. */
		void FinalizeTexture(_In_ Texture &texture, _In_ wstring &&id);
		/* Loads and stages a font from a specific path. */
		void InitializeFont(_In_ Font &font, _In_ const wstring &path, _In_ Task &continuation);
		/* Loads and stages the meshes and materials. */
		void InitializeModel(_In_ Model &model, _In_ const wstring &path, _In_ const DeferredRenderer &deferred, _In_ const LightProbeRenderer &probes);
		/* Stages the contents of the source buffer into the destination buffer and deletes the source buffer once completed. */
		void StageBuffer(_In_ StagingBuffer &source, _In_ Buffer &destination, _In_ PipelineStageFlag dstStage, _In_ AccessFlag access);

	private:
		AssetCache &cache;
		TaskScheduler &scheduler;
		LogicalDevice &device;
		Queue &transferQueue, &graphicsQueue;
	};
}