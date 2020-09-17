#include "Graphics/Vulkan/VulkanInstanceProcedures.h"
#include "Graphics/Vulkan/Loader.h"

#define LOAD_INSTANCE_PROC(name)	VK_LOAD_INSTANCE_PROC(instance, name)

namespace Pu
{
	PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
	PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;
	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkDestroySemaphore vkDestroySemaphore;
	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkQueueSubmit vkQueueSubmit;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
	PFN_vkCmdClearColorImage vkCmdClearColorImage;
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkDestroyRenderPass vkDestroyRenderPass;
	PFN_vkCreateShaderModule vkCreateShaderModule;
	PFN_vkDestroyShaderModule vkDestroyShaderModule;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkDestroyImageView vkDestroyImageView;
	PFN_vkCreateFramebuffer vkCreateFramebuffer;
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
	PFN_vkCreateComputePipelines vkCreateComputePipelines;
	PFN_vkDestroyPipeline vkDestroyPipeline;
	PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
	PFN_vkCmdBindPipeline vkCmdBindPipeline;
	PFN_vkCmdDraw vkCmdDraw;
	PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
	PFN_vkCreateFence vkCreateFence;
	PFN_vkDestroyFence vkDestroyFence;
	PFN_vkGetFenceStatus vkGetFenceStatus;
	PFN_vkResetFences vkResetFences;
	PFN_vkWaitForFences vkWaitForFences;
	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkUnmapMemory vkUnmapMemory;
	PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
	PFN_vkCreateImage vkCreateImage;
	PFN_vkDestroyImage vkDestroyImage;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
	PFN_vkCreateSampler vkCreateSampler;
	PFN_vkDestroySampler vkDestroySampler;
	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
	PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
	PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
	PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
	PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
	PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	PFN_vkCreateQueryPool vkCreateQueryPool;
	PFN_vkDestroyQueryPool vkDestroyQueryPool;
	PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
	PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
	PFN_vkCmdSetViewport vkCmdSetViewport;
	PFN_vkCmdSetScissor vkCmdSetScissor;
	PFN_vkCmdPushConstants vkCmdPushConstants;
	PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
	PFN_vkCmdNextSubpass vkCmdNextSubpass;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;
	PFN_vkCmdBeginQuery vkCmdBeginQuery;
	PFN_vkCmdEndQuery vkCmdEndQuery;
	PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
	PFN_vkCmdBlitImage vkCmdBlitImage;
	PFN_vkCreatePipelineCache vkCreatePipelineCache;
	PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
	PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
	PFN_vkMergePipelineCaches vkMergePipelineCaches;
	PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
	PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
	PFN_vkAcquireFullScreenExclusiveModeEXT vkAcquireFullScreenExclusiveModeEXT;
	PFN_vkReleaseFullScreenExclusiveModeEXT vkReleaseFullScreenExclusiveModeEXT;
	PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
	PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
	PFN_vkResetDescriptorPool vkResetDescriptorPool;
	PFN_vkCmdDispatch vkCmdDispatch;
	PFN_vkGetPipelineExecutableInternalRepresentationsKHR vkGetPipelineExecutableInternalRepresentationsKHR;
	PFN_vkGetPipelineExecutablePropertiesKHR vkGetPipelineExecutablePropertiesKHR;
	PFN_vkGetPipelineExecutableStatisticsKHR vkGetPipelineExecutableStatisticsKHR;

#ifdef _WIN32
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
}

void Pu::vkInit(InstanceHndl instance, const vector<const char*> enabledExtensions)
{
	/* Only has to be called once. */
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	LOAD_INSTANCE_PROC(vkEnumerateInstanceVersion);
	LOAD_INSTANCE_PROC(vkEnumerateInstanceExtensionProperties);
	LOAD_INSTANCE_PROC(vkEnumerateInstanceLayerProperties);
	LOAD_INSTANCE_PROC(vkEnumeratePhysicalDevices);
	LOAD_INSTANCE_PROC(vkCreateInstance);
	LOAD_INSTANCE_PROC(vkDestroyInstance);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceProperties2);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceFeatures2);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
	LOAD_INSTANCE_PROC(vkCreateDevice);
	LOAD_INSTANCE_PROC(vkEnumerateDeviceExtensionProperties);
	LOAD_INSTANCE_PROC(vkDestroyDevice);
	LOAD_INSTANCE_PROC(vkGetDeviceQueue);
	LOAD_INSTANCE_PROC(vkCreateSwapchainKHR);
	LOAD_INSTANCE_PROC(vkDestroySwapchainKHR);
	LOAD_INSTANCE_PROC(vkGetSwapchainImagesKHR);
	LOAD_INSTANCE_PROC(vkAcquireNextImageKHR);
	LOAD_INSTANCE_PROC(vkQueuePresentKHR);
	LOAD_INSTANCE_PROC(vkCreateSemaphore);
	LOAD_INSTANCE_PROC(vkDestroySemaphore);
	LOAD_INSTANCE_PROC(vkCreateCommandPool);
	LOAD_INSTANCE_PROC(vkDestroyCommandPool);
	LOAD_INSTANCE_PROC(vkAllocateCommandBuffers);
	LOAD_INSTANCE_PROC(vkFreeCommandBuffers);
	LOAD_INSTANCE_PROC(vkQueueSubmit);
	LOAD_INSTANCE_PROC(vkBeginCommandBuffer);
	LOAD_INSTANCE_PROC(vkEndCommandBuffer);
	LOAD_INSTANCE_PROC(vkDeviceWaitIdle);
	LOAD_INSTANCE_PROC(vkCmdClearColorImage);
	LOAD_INSTANCE_PROC(vkCmdPipelineBarrier);
	LOAD_INSTANCE_PROC(vkCreateRenderPass);
	LOAD_INSTANCE_PROC(vkDestroyRenderPass);
	LOAD_INSTANCE_PROC(vkCreateShaderModule);
	LOAD_INSTANCE_PROC(vkDestroyShaderModule);
	LOAD_INSTANCE_PROC(vkCreateImageView);
	LOAD_INSTANCE_PROC(vkDestroyImageView);
	LOAD_INSTANCE_PROC(vkCreateFramebuffer);
	LOAD_INSTANCE_PROC(vkDestroyFramebuffer);
	LOAD_INSTANCE_PROC(vkCreatePipelineLayout);
	LOAD_INSTANCE_PROC(vkDestroyPipelineLayout);
	LOAD_INSTANCE_PROC(vkCreateGraphicsPipelines);
	LOAD_INSTANCE_PROC(vkCreateComputePipelines);
	LOAD_INSTANCE_PROC(vkDestroyPipeline);
	LOAD_INSTANCE_PROC(vkCmdBeginRenderPass);
	LOAD_INSTANCE_PROC(vkCmdBindPipeline);
	LOAD_INSTANCE_PROC(vkCmdDraw);
	LOAD_INSTANCE_PROC(vkCmdEndRenderPass);
	LOAD_INSTANCE_PROC(vkCreateFence);
	LOAD_INSTANCE_PROC(vkDestroyFence);
	LOAD_INSTANCE_PROC(vkGetFenceStatus);
	LOAD_INSTANCE_PROC(vkResetFences);
	LOAD_INSTANCE_PROC(vkWaitForFences);
	LOAD_INSTANCE_PROC(vkCreateBuffer);
	LOAD_INSTANCE_PROC(vkDestroyBuffer);
	LOAD_INSTANCE_PROC(vkGetBufferMemoryRequirements);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceMemoryProperties);
	LOAD_INSTANCE_PROC(vkAllocateMemory);
	LOAD_INSTANCE_PROC(vkFreeMemory);
	LOAD_INSTANCE_PROC(vkBindBufferMemory);
	LOAD_INSTANCE_PROC(vkMapMemory);
	LOAD_INSTANCE_PROC(vkFlushMappedMemoryRanges);
	LOAD_INSTANCE_PROC(vkUnmapMemory);
	LOAD_INSTANCE_PROC(vkCmdBindVertexBuffers);
	LOAD_INSTANCE_PROC(vkCmdCopyBuffer);
	LOAD_INSTANCE_PROC(vkCreateImage);
	LOAD_INSTANCE_PROC(vkDestroyImage);
	LOAD_INSTANCE_PROC(vkGetImageMemoryRequirements);
	LOAD_INSTANCE_PROC(vkBindImageMemory);
	LOAD_INSTANCE_PROC(vkCmdCopyBufferToImage);
	LOAD_INSTANCE_PROC(vkCreateSampler);
	LOAD_INSTANCE_PROC(vkDestroySampler);
	LOAD_INSTANCE_PROC(vkCreateDescriptorPool);
	LOAD_INSTANCE_PROC(vkDestroyDescriptorPool);
	LOAD_INSTANCE_PROC(vkCreateDescriptorSetLayout);
	LOAD_INSTANCE_PROC(vkDestroyDescriptorSetLayout);
	LOAD_INSTANCE_PROC(vkAllocateDescriptorSets);
	LOAD_INSTANCE_PROC(vkFreeDescriptorSets);
	LOAD_INSTANCE_PROC(vkUpdateDescriptorSets);
	LOAD_INSTANCE_PROC(vkCmdBindDescriptorSets);
	LOAD_INSTANCE_PROC(vkCmdBindIndexBuffer);
	LOAD_INSTANCE_PROC(vkCmdDrawIndexed);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceFormatProperties);
	LOAD_INSTANCE_PROC(vkGetPhysicalDeviceImageFormatProperties);
	LOAD_INSTANCE_PROC(vkCmdCopyImageToBuffer);
	LOAD_INSTANCE_PROC(vkCreateQueryPool);
	LOAD_INSTANCE_PROC(vkDestroyQueryPool);
	LOAD_INSTANCE_PROC(vkGetQueryPoolResults);
	LOAD_INSTANCE_PROC(vkCmdWriteTimestamp);
	LOAD_INSTANCE_PROC(vkCmdSetViewport);
	LOAD_INSTANCE_PROC(vkCmdSetScissor);
	LOAD_INSTANCE_PROC(vkCmdPushConstants);
	LOAD_INSTANCE_PROC(vkCmdSetLineWidth);
	LOAD_INSTANCE_PROC(vkCmdNextSubpass);
	LOAD_INSTANCE_PROC(vkQueueWaitIdle);
	LOAD_INSTANCE_PROC(vkCmdBeginQuery);
	LOAD_INSTANCE_PROC(vkCmdEndQuery);
	LOAD_INSTANCE_PROC(vkGetDeviceMemoryCommitment);
	LOAD_INSTANCE_PROC(vkCmdBlitImage);
	LOAD_INSTANCE_PROC(vkCreatePipelineCache);
	LOAD_INSTANCE_PROC(vkDestroyPipelineCache);
	LOAD_INSTANCE_PROC(vkGetPipelineCacheData);
	LOAD_INSTANCE_PROC(vkMergePipelineCaches);
	LOAD_INSTANCE_PROC(vkCmdResetQueryPool);
	LOAD_INSTANCE_PROC(vkCmdDrawIndirect);
	LOAD_INSTANCE_PROC(vkCmdDrawIndexedIndirect);
	LOAD_INSTANCE_PROC(vkResetDescriptorPool);
	LOAD_INSTANCE_PROC(vkCmdDispatch);

	if (enabledExtensions.contains(u8"VK_EXT_debug_utils"))
	{
		LOAD_INSTANCE_PROC(vkCreateDebugUtilsMessengerEXT);
		LOAD_INSTANCE_PROC(vkDestroyDebugUtilsMessengerEXT);
		LOAD_INSTANCE_PROC(vkSetDebugUtilsObjectNameEXT);
		LOAD_INSTANCE_PROC(vkQueueBeginDebugUtilsLabelEXT);
		LOAD_INSTANCE_PROC(vkQueueEndDebugUtilsLabelEXT);
		LOAD_INSTANCE_PROC(vkCmdBeginDebugUtilsLabelEXT);
		LOAD_INSTANCE_PROC(vkCmdEndDebugUtilsLabelEXT);
	}

	if (enabledExtensions.contains(u8"VK_KHR_surface"))
	{
		LOAD_INSTANCE_PROC(vkDestroySurfaceKHR);
		LOAD_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
		LOAD_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		LOAD_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
		LOAD_INSTANCE_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
	}

	if (enabledExtensions.contains(u8"VK_EXT_full_screen_exclusive"))
	{
		LOAD_INSTANCE_PROC(vkAcquireFullScreenExclusiveModeEXT);
		LOAD_INSTANCE_PROC(vkReleaseFullScreenExclusiveModeEXT);
	}

	if (enabledExtensions.contains(u8"VK_KHR_get_physical_device_properties2"))
	{
		LOAD_INSTANCE_PROC(vkGetPhysicalDeviceMemoryProperties2);
	}

	if (enabledExtensions.contains(u8"VK_KHR_pipeline_executable_properties"))
	{
		LOAD_INSTANCE_PROC(vkGetPipelineExecutableInternalRepresentationsKHR);
		LOAD_INSTANCE_PROC(vkGetPipelineExecutablePropertiesKHR);
		LOAD_INSTANCE_PROC(vkGetPipelineExecutableStatisticsKHR);
	}

#ifdef _WIN32
	if (enabledExtensions.contains(u8"VK_KHR_win32_surface"))
	{
		LOAD_INSTANCE_PROC(vkCreateWin32SurfaceKHR);
	}
#endif
}