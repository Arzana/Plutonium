#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

Pu::LogicalDevice::LogicalDevice(LogicalDevice && value)
	: parent(value.parent), hndl(value.hndl), vkDestroyDevice(value.vkDestroyDevice)
{
	value.hndl = nullptr;
	value.vkDestroyDevice = nullptr;
}

LogicalDevice & Pu::LogicalDevice::operator=(LogicalDevice && other)
{
	if (this != &other)
	{
		Destory();
		parent = std::move(other.parent);
		hndl = other.hndl;
		vkDestroyDevice = other.vkDestroyDevice;

		other.hndl = nullptr;
		other.vkDestroyDevice = nullptr;
	}

	return *this;
}

Pu::LogicalDevice::LogicalDevice(PhysicalDevice & parent, DeviceHndl hndl, uint32 queueCreateInfoCount, const DeviceQueueCreateInfo * queueCreateInfos)
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
				storage.push_back(Queue(*this, queue));
				queues.emplace(cur->QueueFamilyIndex, std::move(storage));
			}
			else it->second.push_back(Queue(*this, queue));
		}
	}
}

void Pu::LogicalDevice::LoadDeviceProcs(void)
{
	/* Logical device related functions. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkDestroyDevice);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkGetDeviceQueue);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkQueueSubmit);

	/* Swapchain related functions. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkCreateSwapchainKHR);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkDestroySwapchainKHR);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkGetSwapchainImagesKHR);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkAcquireNextImageKHR);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkQueuePresentKHR);

	/* Semaphore related functions. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkCreateSemaphore);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkDestroySemaphore);

	/* Command pool related functions. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkCreateCommandPool);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkDestroyCommandPool);

	/* Command buffer related functions. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkAllocateCommandBuffers);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkFreeCommandBuffers);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkBeginCommandBuffer);

	/* Commands. */
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkCmdClearColorImage);
	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, vkCmdPipelineBarrier);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		vkDestroyDevice(hndl, nullptr);
	}
}