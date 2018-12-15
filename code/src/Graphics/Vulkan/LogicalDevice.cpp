#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

/* Ease of use define for loading within the logical device class. */
#define LOAD_DEVICE_PROC(name)	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, name)

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
		/* Make sure to reload the device procs. */
		LoadDeviceProcs();

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
	LOAD_DEVICE_PROC(vkDestroyDevice);
	LOAD_DEVICE_PROC(vkGetDeviceQueue);
	LOAD_DEVICE_PROC(vkQueueSubmit);
	LOAD_DEVICE_PROC(vkDeviceWaitIdle);

	/* Swapchain related functions. */
	if (parent.IsExtensionSupported(u8"VK_KHR_swapchain"))
	{
		LOAD_DEVICE_PROC(vkCreateSwapchainKHR);
		LOAD_DEVICE_PROC(vkDestroySwapchainKHR);
		LOAD_DEVICE_PROC(vkGetSwapchainImagesKHR);
		LOAD_DEVICE_PROC(vkAcquireNextImageKHR);
		LOAD_DEVICE_PROC(vkQueuePresentKHR);
	}
	else Log::Warning("%s doesn't support required swapchain extension!", parent.GetName());

	/* Semaphore related functions. */
	LOAD_DEVICE_PROC(vkCreateSemaphore);
	LOAD_DEVICE_PROC(vkDestroySemaphore);

	/* Command pool related functions. */
	LOAD_DEVICE_PROC(vkCreateCommandPool);
	LOAD_DEVICE_PROC(vkDestroyCommandPool);

	/* Command buffer related functions. */
	LOAD_DEVICE_PROC(vkAllocateCommandBuffers);
	LOAD_DEVICE_PROC(vkFreeCommandBuffers);
	LOAD_DEVICE_PROC(vkBeginCommandBuffer);
	LOAD_DEVICE_PROC(vkEndCommandBuffer);

	/* Commands. */
	LOAD_DEVICE_PROC(vkCmdClearColorImage);
	LOAD_DEVICE_PROC(vkCmdPipelineBarrier);
	
	/* Render pass related functions. */
	LOAD_DEVICE_PROC(vkCreateRenderPass);
	LOAD_DEVICE_PROC(vkDestroyRenderPass);

	/* Shader module related functions. */
	LOAD_DEVICE_PROC(vkCreateShaderModule);
	LOAD_DEVICE_PROC(vkDestroyShaderModule);

	/* Image view related functions. */
	LOAD_DEVICE_PROC(vkCreateImageView);
	LOAD_DEVICE_PROC(vkDestroyImageView);

	/* Framebuffer related functions. */
	LOAD_DEVICE_PROC(vkCreateFramebuffer);
	LOAD_DEVICE_PROC(vkDestroyFramebuffer);

	/* Graphics pipeline related functions. */
	LOAD_DEVICE_PROC(vkCreatePipelineLayout);
	LOAD_DEVICE_PROC(vkDestroyPipelineLayout);
	LOAD_DEVICE_PROC(vkCreateGraphicsPipelines);
	LOAD_DEVICE_PROC(vkDestroyPipeline);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		VK_VALIDATE(vkDeviceWaitIdle(hndl), PFN_vkDeviceWaitIdle);
		vkDestroyDevice(hndl, nullptr);
	}
}