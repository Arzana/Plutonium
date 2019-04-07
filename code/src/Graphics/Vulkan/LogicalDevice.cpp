#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

/* Ease of use define for loading within the logical device class. */
#define LOAD_DEVICE_PROC(name)	VK_LOAD_DEVICE_PROC(parent.parent.hndl, hndl, name)

Pu::LogicalDevice::LogicalDevice(LogicalDevice && value)
	: parent(value.parent), hndl(value.hndl), queues(std::move(value.queues)), 
	graphicsQueueFamily(value.graphicsQueueFamily), transferQueueFamily(value.transferQueueFamily)
{
	/* Make sure to reload the device procs. */
	LoadDeviceProcs();

	value.hndl = nullptr;
}

LogicalDevice & Pu::LogicalDevice::operator=(LogicalDevice && other)
{
	if (this != &other)
	{
		Destory();
		parent = std::move(other.parent);
		queues = std::move(other.queues);
		graphicsQueueFamily = other.graphicsQueueFamily;
		transferQueueFamily = other.transferQueueFamily;
		hndl = other.hndl;

		/* Make sure to reload the device procs. */
		LoadDeviceProcs();

		other.hndl = nullptr;
	}

	return *this;
}

Pu::LogicalDevice::LogicalDevice(PhysicalDevice & parent, DeviceHndl hndl, uint32 queueCreateInfoCount, const DeviceQueueCreateInfo * queueCreateInfos)
	: parent(parent), hndl(hndl), graphicsQueueFamily(0), transferQueueFamily(0)
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
				storage.push_back(Queue(*this, queue, cur->QueueFamilyIndex));
				queues.emplace(cur->QueueFamilyIndex, std::move(storage));
			}
			else it->second.push_back(Queue(*this, queue, cur->QueueFamilyIndex));
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

	/* Command buffer/pool related functions. */
	LOAD_DEVICE_PROC(vkCreateCommandPool);
	LOAD_DEVICE_PROC(vkDestroyCommandPool);
	LOAD_DEVICE_PROC(vkAllocateCommandBuffers);
	LOAD_DEVICE_PROC(vkFreeCommandBuffers);
	LOAD_DEVICE_PROC(vkBeginCommandBuffer);
	LOAD_DEVICE_PROC(vkEndCommandBuffer);

	/* Commands. */
	LOAD_DEVICE_PROC(vkCmdClearColorImage);
	LOAD_DEVICE_PROC(vkCmdPipelineBarrier);
	LOAD_DEVICE_PROC(vkCmdBeginRenderPass);
	LOAD_DEVICE_PROC(vkCmdBindPipeline);
	LOAD_DEVICE_PROC(vkCmdDraw);
	LOAD_DEVICE_PROC(vkCmdEndRenderPass);
	LOAD_DEVICE_PROC(vkCmdBindVertexBuffers);
	LOAD_DEVICE_PROC(vkCmdCopyBuffer);
	LOAD_DEVICE_PROC(vkCmdCopyBufferToImage);
	LOAD_DEVICE_PROC(vkCmdBindDescriptorSets);
	LOAD_DEVICE_PROC(vkCmdBindIndexBuffer);
	LOAD_DEVICE_PROC(vkCmdDrawIndexed);
	LOAD_DEVICE_PROC(vkCmdCopyImageToBuffer);
	
	/* Render pass related functions. */
	LOAD_DEVICE_PROC(vkCreateRenderPass);
	LOAD_DEVICE_PROC(vkDestroyRenderPass);

	/* Shader module related functions. */
	LOAD_DEVICE_PROC(vkCreateShaderModule);
	LOAD_DEVICE_PROC(vkDestroyShaderModule);

	/* Image related functions. */
	LOAD_DEVICE_PROC(vkCreateImage);
	LOAD_DEVICE_PROC(vkDestroyImage);
	LOAD_DEVICE_PROC(vkGetImageMemoryRequirements);
	LOAD_DEVICE_PROC(vkBindImageMemory);
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

	/* Fence related functions. */
	LOAD_DEVICE_PROC(vkCreateFence);
	LOAD_DEVICE_PROC(vkDestroyFence);
	LOAD_DEVICE_PROC(vkGetFenceStatus);
	LOAD_DEVICE_PROC(vkResetFences);
	LOAD_DEVICE_PROC(vkWaitForFences);

	/* Buffer related functions. */
	LOAD_DEVICE_PROC(vkCreateBuffer);
	LOAD_DEVICE_PROC(vkDestroyBuffer);
	LOAD_DEVICE_PROC(vkGetBufferMemoryRequirements);
	LOAD_DEVICE_PROC(vkAllocateMemory);
	LOAD_DEVICE_PROC(vkFreeMemory);
	LOAD_DEVICE_PROC(vkBindBufferMemory);
	LOAD_DEVICE_PROC(vkMapMemory);
	LOAD_DEVICE_PROC(vkFlushMappedMemoryRanges);
	LOAD_DEVICE_PROC(vkUnmapMemory);

	/* Sampler related functions. */
	LOAD_DEVICE_PROC(vkCreateSampler);
	LOAD_DEVICE_PROC(vkDestroySampler);

	/* Descriptor pool/set related functions. */
	LOAD_DEVICE_PROC(vkCreateDescriptorPool);
	LOAD_DEVICE_PROC(vkDestroyDescriptorPool);
	LOAD_DEVICE_PROC(vkAllocateDescriptorSets);
	LOAD_DEVICE_PROC(vkFreeDescriptorSets);
	LOAD_DEVICE_PROC(vkCreateDescriptorSetLayout);
	LOAD_DEVICE_PROC(vkDestroyDescriptorSetLayout);
	LOAD_DEVICE_PROC(vkUpdateDescriptorSets);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		VK_VALIDATE(vkDeviceWaitIdle(hndl), PFN_vkDeviceWaitIdle);
		vkDestroyDevice(hndl, nullptr);
	}
}