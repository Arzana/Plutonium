#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"

using namespace Pu;

Pu::LogicalDevice::LogicalDevice(LogicalDevice && value)
	: parent(value.parent), hndl(value.hndl), vkDestroyDevice(value.vkDestroyDevice)
{
	value.parent = nullptr;
	value.hndl = nullptr;
	value.vkDestroyDevice = nullptr;
}

LogicalDevice & Pu::LogicalDevice::operator=(LogicalDevice && other)
{
	if (this != &other)
	{
		Destory();
		parent = other.parent;
		hndl = other.hndl;
		vkDestroyDevice = other.vkDestroyDevice;

		other.parent = nullptr;
		other.hndl = nullptr;
		other.vkDestroyDevice = nullptr;
	}

	return *this;
}

Pu::LogicalDevice::LogicalDevice(InstanceHndl parent, DeviceHndl hndl, uint32 queueCreateInfoCount, const DeviceQueueCreateInfo * queueCreateInfos)
	: parent(parent), hndl(hndl)
{
	LoadDeviceProcs();

	/* Preload all queues that where created with the logical device. */
	for (uint32 i = 0; i < queueCreateInfoCount; i++)
	{
		const DeviceQueueCreateInfo *cur = queueCreateInfos + i;

		for (uint32 j = 0; j < cur->Count; j++)
		{
			QueueHndl queue;
			vkGetDeviceQueue(hndl, cur->QueueFamilyIndex, j, &queue);

			std::map<uint32, vector<Queue>>::iterator it = queues.find(cur->QueueFamilyIndex);
			if (it == queues.end())
			{
				vector<Queue> storage;
				storage.push_back(Queue(queue));
				queues.emplace(cur->QueueFamilyIndex, std::move(storage));
			}
			else it->second.push_back(Queue(queue));
		}
	}
}

void Pu::LogicalDevice::LoadDeviceProcs(void)
{
	VK_LOAD_DEVICE_PROC(parent, hndl, vkDestroyDevice);
	VK_LOAD_DEVICE_PROC(parent, hndl, vkGetDeviceQueue);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		vkDestroyDevice(hndl, nullptr);
	}
}