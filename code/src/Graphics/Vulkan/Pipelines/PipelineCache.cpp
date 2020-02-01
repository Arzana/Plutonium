#include "Graphics/Vulkan/Pipelines/PipelineCache.h"
#include "Streams/FileWriter.h"
#include "Streams/FileReader.h"
#include "Streams/BinaryReader.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::PipelineCache::PipelineCache(LogicalDevice & device, TaskScheduler & scheduler)
	: device(&device), scheduler(&scheduler), fromFile(false)
{
	/* Simply allocate an empty pipeline cache. */
	const PipelineCacheCreateInfo createInfo;
	VK_VALIDATE(device.vkCreatePipelineCache(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreatePipelineCache);
}

Pu::PipelineCache::PipelineCache(LogicalDevice & device, TaskScheduler & scheduler, const wstring & path)
	: device(&device), scheduler(&scheduler), hndl(nullptr)
{
	class LoadTask
		: public Task
	{
	public:
		LoadTask(LogicalDevice *device, PipelineCache *cache, const wstring &path)
			: device(device), cache(cache), path(path)
		{}

		virtual Result Execute(void)
		{
			PipelineCacheCreateInfo createInfo;

			/* Open the cache file (if it exists). */
			FileReader raw{ path };
			if (raw.IsOpen())
			{
				const string src = raw.ReadToEnd();
				BinaryReader reader{ src.c_str(), src.size() };
				bool good = true;

				/* Validate the cache header (read header size). */
				if (reader.ReadUInt32() < 16 + UUIDSize)
				{
					Log::Error("Unable to use pipeline cache '%ls' (invalid header length)!", path.fileName().c_str());
					good = false;
				}

				/* Validate the cache header version. */
				if (reader.ReadUInt32() != _CrtEnum2Int(PipelineCacheHeaderVersion::One))
				{
					Log::Error("Unable to use pipeline cache: '%ls' (invalid header version)!", path.fileName().c_str());
					good = false;
				}

				/* Make sure that this cache was created using the same physical device. */
				if (reader.ReadUInt32() != device->parent->GetVendorID())
				{
					Log::Error("Unable to use pipeline cache: '%ls' (physical device mismatch)!", path.fileName().c_str());
					good = false;
				}

				/* Make sure that this cache was created using the same logical device. */
				if (reader.ReadUInt32() != device->parent->GetDeviceID())
				{
					Log::Error("Unable to use pipeline cache: '%ls' (logical device mismatch)!", path.fileName().c_str());
					good = false;
				}

				/* Make sure that the cache UUID is the same. */
				uint8 uuid[UUIDSize];
				reader.Read(uuid, 0, UUIDSize);
				if (memcmp(uuid, device->parent->properties.PipelineCacheUUID, UUIDSize))
				{
					Log::Error("Unable to use pipeline cache: '%ls' (cache UUID mismatch)!", path.fileName().c_str());
					good = false;
				}

				/* Set the actual cache data if the cache passed the checks. */
				if (good)
				{
					createInfo.InitialDataSize = src.size();
					createInfo.InitialData = src.c_str();
					cache->fromFile = true;
				}
			}

			/* Always allocate a pipeline cache. */
			VK_VALIDATE(device->vkCreatePipelineCache(device->hndl, &createInfo, nullptr, &cache->hndl), PFN_vkCreatePipelineCache);
			return Result::AutoDelete();
		}

	private:
		PipelineCache *cache;
		LogicalDevice *device;
		wstring path;
	};

	LoadTask *task = new LoadTask(&device, this, path);
	scheduler.Spawn(*task);
}

Pu::PipelineCache::PipelineCache(PipelineCache && value)
	: device(value.device), scheduler(value.scheduler), hndl(value.hndl), fromFile(value.fromFile)
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
		fromFile = other.fromFile;
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::PipelineCache::Store(const wstring & path) const
{
	class StoreTask
		: public Task
	{
	public:
		StoreTask(LogicalDevice *device, PipelineCacheHndl hndl, const wstring &path)
			: device(device), hndl(hndl), path(path)
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