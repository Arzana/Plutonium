#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

/* Ease of use define for loading within the logical device class. */
#define LOAD_DEVICE_PROC(name)	VK_LOAD_DEVICE_PROC(parent->parent->hndl, hndl, name)

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
		parent = other.parent;
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

Pu::LogicalDevice::LogicalDevice(PhysicalDevice & parent, DeviceHndl hndl, const DeviceCreateInfo &createInfo)
	: parent(&parent), hndl(hndl), graphicsQueueFamily(0), transferQueueFamily(0)
{
	/* Copy all the enabled extensions, so we check them later. */
	enabledExtensions.reserve(createInfo.EnabledExtensionCount);
	for (uint32 i = 0; i < createInfo.EnabledExtensionCount; i++) enabledExtensions.emplace_back(createInfo.EnabledExtensionNames[i]);

	LoadDeviceProcs();

	/* Preload all queues that where created with the logical device. */
	for (uint32 i = 0; i < createInfo.QueueCreateInfoCount; i++)
	{
		const DeviceQueueCreateInfo &cur = createInfo.QueueCreateInfos[i];

		for (uint32 j = 0; j < cur.Count; j++)
		{
			QueueHndl queue;
			vkGetDeviceQueue(hndl, cur.QueueFamilyIndex, j, &queue);

			std::map<uint32, vector<Queue>>::iterator it = queues.find(cur.QueueFamilyIndex);
			if (it == queues.end())
			{
				vector<Queue> storage;
				storage.push_back(Queue(*this, queue, cur.QueueFamilyIndex));
				queues.emplace(cur.QueueFamilyIndex, std::move(storage));
			}
			else it->second.emplace_back(Queue(*this, queue, cur.QueueFamilyIndex));
		}
	}
}

#ifdef _DEBUG
void Pu::LogicalDevice::SetDebugName(ObjectType type, const void * handle, const string & name)
{
	if (parent->parent->IsExtensionEnabled(u8"VK_EXT_debug_utils"))
	{
		const DebugUtilsObjectNameInfo info(type, reinterpret_cast<uint64>(handle), name.c_str());
		VK_VALIDATE(parent->parent->vkSetDebugUtilsObjectNameEXT(hndl, &info), PFN_vkSetDebugUtilsObjectNameEXT);
	}
}

void Pu::LogicalDevice::BeginQueueLabel(QueueHndl queue, const DebugUtilsLabel & label)
{
	if (parent->parent->IsExtensionEnabled(u8"VK_EXT_debug_utils"))
	{
		parent->parent->vkQueueBeginDebugUtilsLabelEXT(queue, &label);
	}
}

void Pu::LogicalDevice::EndQueueLabel(QueueHndl queue)
{
	if (parent->parent->IsExtensionEnabled(u8"VK_EXT_debug_utils"))
	{
		parent->parent->vkQueueEndDebugUtilsLabelEXT(queue);
	}
}

void Pu::LogicalDevice::BeginCommandBufferLabel(CommandBufferHndl commandBuffer, const DebugUtilsLabel & label)
{
	if (parent->parent->IsExtensionEnabled(u8"VK_EXT_debug_utils"))
	{
		parent->parent->vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &label);
	}
}

void Pu::LogicalDevice::EndCommandBufferLabel(CommandBufferHndl commandBuffer)
{
	if (parent->parent->IsExtensionEnabled(u8"VK_EXT_debug_utils"))
	{
		parent->parent->vkCmdEndDebugUtilsLabelEXT(commandBuffer);
	}
}
#endif

void Pu::LogicalDevice::SetQueues(uint32 graphics, uint32 transfer)
{
	graphicsQueueFamily = graphics;
	transferQueueFamily = transfer;

#ifdef _DEBUG
	for (Queue &cur : queues[graphics]) SetDebugName(ObjectType::Queue, cur.hndl, u8"Graphics Queue");
	for (Queue &cur : queues[transfer]) SetDebugName(ObjectType::Queue, cur.hndl, u8"Transfer Queue");
#endif
}

void Pu::LogicalDevice::LoadDeviceProcs(void)
{
	/* Logical device related functions. */
	LOAD_DEVICE_PROC(vkDestroyDevice);
	LOAD_DEVICE_PROC(vkGetDeviceQueue);
	LOAD_DEVICE_PROC(vkQueueSubmit);
	LOAD_DEVICE_PROC(vkDeviceWaitIdle);
	LOAD_DEVICE_PROC(vkQueueWaitIdle);

	/* Swapchain related functions. */
	if (IsExtensionEnabled(u8"VK_KHR_swapchain"))
	{
		LOAD_DEVICE_PROC(vkCreateSwapchainKHR);
		LOAD_DEVICE_PROC(vkDestroySwapchainKHR);
		LOAD_DEVICE_PROC(vkGetSwapchainImagesKHR);
		LOAD_DEVICE_PROC(vkAcquireNextImageKHR);
		LOAD_DEVICE_PROC(vkQueuePresentKHR);
	}

	/* Functions related to the full-screen extension. */
	if (IsExtensionEnabled(u8"VK_EXT_full_screen_exclusive"))
	{
		LOAD_DEVICE_PROC(vkAcquireFullScreenExclusiveModeEXT);
		LOAD_DEVICE_PROC(vkReleaseFullScreenExclusiveModeEXT);
	}

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
	LOAD_DEVICE_PROC(vkCmdWriteTimestamp);
	LOAD_DEVICE_PROC(vkCmdSetViewport);
	LOAD_DEVICE_PROC(vkCmdSetScissor);
	LOAD_DEVICE_PROC(vkCmdPushConstants);
	LOAD_DEVICE_PROC(vkCmdSetLineWidth);
	LOAD_DEVICE_PROC(vkCmdNextSubpass);
	LOAD_DEVICE_PROC(vkCmdBeginQuery);
	LOAD_DEVICE_PROC(vkCmdEndQuery);
	LOAD_DEVICE_PROC(vkCmdBlitImage);
	LOAD_DEVICE_PROC(vkCmdResetQueryPool);
	LOAD_DEVICE_PROC(vkCmdDrawIndirect);
	LOAD_DEVICE_PROC(vkCmdDrawIndexedIndirect);
	LOAD_DEVICE_PROC(vkCmdDispatch);
	
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

	/* Pipeline related functions. */
	LOAD_DEVICE_PROC(vkCreatePipelineLayout);
	LOAD_DEVICE_PROC(vkDestroyPipelineLayout);
	LOAD_DEVICE_PROC(vkCreateGraphicsPipelines);
	LOAD_DEVICE_PROC(vkCreateComputePipelines);
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
	LOAD_DEVICE_PROC(vkGetDeviceMemoryCommitment);
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
	LOAD_DEVICE_PROC(vkResetDescriptorPool);
	LOAD_DEVICE_PROC(vkAllocateDescriptorSets);
	LOAD_DEVICE_PROC(vkFreeDescriptorSets);
	LOAD_DEVICE_PROC(vkCreateDescriptorSetLayout);
	LOAD_DEVICE_PROC(vkDestroyDescriptorSetLayout);
	LOAD_DEVICE_PROC(vkUpdateDescriptorSets);

	/* Query related functions. */
	LOAD_DEVICE_PROC(vkCreateQueryPool);
	LOAD_DEVICE_PROC(vkDestroyQueryPool);
	LOAD_DEVICE_PROC(vkGetQueryPoolResults);

	/* Pipeline cache related functions. */
	LOAD_DEVICE_PROC(vkCreatePipelineCache);
	LOAD_DEVICE_PROC(vkDestroyPipelineCache);
	LOAD_DEVICE_PROC(vkGetPipelineCacheData);
	LOAD_DEVICE_PROC(vkMergePipelineCaches);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		VK_VALIDATE(vkDeviceWaitIdle(hndl), PFN_vkDeviceWaitIdle);
		vkDestroyDevice(hndl, nullptr);
	}
}