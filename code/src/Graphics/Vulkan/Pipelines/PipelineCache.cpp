#include "Graphics/Vulkan/Pipelines/PipelineCache.h"
#include "Streams/FileWriter.h"
#include "Streams/FileReader.h"
#include "Streams/BinaryReader.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::PipelineCache::PipelineCache(LogicalDevice & device, TaskScheduler & scheduler, const wstring & path)
	: device(&device), scheduler(&scheduler), hndl(nullptr), path(path)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(LogicalDevice *device, PipelineCache *cache)
			: Task("Load Pipeline Cache"), device(device), cache(cache), good(true)
		{}

		virtual Result Execute(void)
		{
			PipelineCacheCreateInfo createInfo;

			/* Open the cache file (if it exists). */
			FileReader raw{ cache->path, false };
			if (raw.IsOpen())
			{
				const string src = raw.ReadToEnd();
				BinaryReader reader{ src.c_str(), src.size() };

				/* Validate the cache header. */
				if (reader.ReadUInt32() < 16 + UUIDSize) LogFailure("invalid header length");
				if (reader.ReadUInt32() != _CrtEnum2Int(PipelineCacheHeaderVersion::One)) LogFailure("invalid header version");
				if (reader.ReadUInt32() != device->parent->GetVendorID()) LogFailure("physical device mismatch");
				if (reader.ReadUInt32() != device->parent->GetDeviceID()) LogFailure("logical device mismatch");

				uint8 uuid[UUIDSize];
				reader.Read(uuid, 0, UUIDSize);
				if (memcmp(uuid, device->parent->properties.PipelineCacheUUID, UUIDSize)) LogFailure("cache UUID mismatch");

				/* Set the actual cache data if the cache passed the checks. */
				if (good)
				{
					createInfo.InitialDataSize = src.size();
					createInfo.InitialData = src.c_str();
				}
			}

			/* Always allocate a pipeline cache. */
			VK_VALIDATE(device->vkCreatePipelineCache(device->hndl, &createInfo, nullptr, &cache->hndl), PFN_vkCreatePipelineCache);
			return Result::AutoDelete();
		}

	private:
		PipelineCache *cache;
		LogicalDevice *device;
		bool good;

		void LogFailure(const char *msg)
		{
			Log::Error("Unable to use pipeline cache: '%ls' (%s)!", cache->path.fileName().c_str(), msg);
			good = false;
		}
	};

	LoadTask *task = new LoadTask(&device, this);
	scheduler.Spawn(*task);
}

Pu::PipelineCache::PipelineCache(PipelineCache && value)
	: device(value.device), scheduler(value.scheduler), hndl(value.hndl), path(value.path)
{
	value.hndl = nullptr;
}

Pu::PipelineCache & Pu::PipelineCache::operator=(PipelineCache && other)
{
	if (this != &other)
	{
		Destroy();
		device = other.device;
		scheduler = other.scheduler;
		hndl = other.hndl;
		path = other.path;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::PipelineCache::Store(void) const
{
	class StoreTask
		: public Task
	{
	public:
		StoreTask(LogicalDevice *device, PipelineCacheHndl hndl, const wstring &path)
			: Task("Save Pipeline Cache"), device(device), hndl(hndl), path(path)
		{}

		virtual Result Execute(void)
		{
			/* Get the size of the cache store. */
			size_t size = 0;
			VK_VALIDATE(device->vkGetPipelineCacheData(device->hndl, hndl, &size, nullptr), PFN_vkGetPipelineCacheData);

			/* Skip saving if no cache store is available. */
			if (size)
			{
				/* Get the actual data store. */
				byte *data = reinterpret_cast<byte*>(malloc(size));
				VK_VALIDATE(device->vkGetPipelineCacheData(device->hndl, hndl, &size, data), PFN_vkGetPipelineCacheData);

				/* Save the data  */
				FileWriter writer{ path };
				if (writer.IsCreated())
				{
					writer.AutoFlush = true;
					writer.Write(data, 0, size);
				}

				/* Release the temporary buffer. */
				free(data);
			}

			return Result::AutoDelete();
		}

	private:
		LogicalDevice *device;
		PipelineCacheHndl hndl;
		wstring path;
	};

	StoreTask *task = new StoreTask(device, hndl, path);
	scheduler->Spawn(*task);
}

void Pu::PipelineCache::Merge(const PipelineCache & other)
{
	VK_VALIDATE(device->vkMergePipelineCaches(device->hndl, hndl, 1, &other.hndl), PFN_vkMergePipelineCaches);
}

void Pu::PipelineCache::Destroy(void)
{
	if (hndl) device->vkDestroyPipelineCache(device->hndl, hndl, nullptr);
}