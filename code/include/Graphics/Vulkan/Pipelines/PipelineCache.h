#pragma once
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Core/Threading/Tasks/Scheduler.h"

namespace Pu
{
	/* Defines a helper object for creating and loading pipeline caches. */
	class PipelineCache
	{
	public:
		/* Initializes a new instance of a pipeline cache object from a disk cache file. */
		PipelineCache(_In_ LogicalDevice &device, _In_ TaskScheduler &scheduler, _In_ const wstring &path);
		PipelineCache(_In_ const PipelineCache&) = delete;
		/* Move constructor. */
		PipelineCache(_In_ PipelineCache &&value);
		/* Releases the resources allocated by the pipeline cache. */
		~PipelineCache(void)
		{
			Destroy();
		}

		_Check_return_ PipelineCache& operator =(_In_ const PipelineCache&) = delete;
		/* Move assignment. */
		_Check_return_ PipelineCache& operator =(_In_ PipelineCache &&other);

		/* Stores the pipeline cache to disk. */
		void Store(void) const;
		/* Merges the other pipeline cache into this pipeline cache. */
		void Merge(_In_ const PipelineCache &other);

	private:
		PipelineCacheHndl hndl;
		LogicalDevice *device;
		TaskScheduler *scheduler;
		wstring path;

		void Destroy(void);
	};
}