/*
This file contains instance level function calls for Vulkan.
These calls have to be initialized using Pu::vkInit();
It is not recommended to use these functions as they will call dispatch code
on platforms that have multiple Vulkan capable devices.
For normal use of Vulkan use the Plutonium structures, but if instance level access is needed than this can be used.
*/

#pragma once
#include "VulkanProcedres.h"
#include "Core/Collections/vector.h"

#define EXT_INSTANCE_PROC(proc)	extern PFN_##proc proc

namespace Pu
{
	EXT_INSTANCE_PROC(vkEnumerateInstanceVersion);
	EXT_INSTANCE_PROC(vkEnumerateInstanceExtensionProperties);
	EXT_INSTANCE_PROC(vkEnumerateInstanceLayerProperties);
	EXT_INSTANCE_PROC(vkEnumeratePhysicalDevices);
	EXT_INSTANCE_PROC(vkCreateInstance);
	EXT_INSTANCE_PROC(vkDestroyInstance);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceProperties2);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceFeatures);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
	EXT_INSTANCE_PROC(vkCreateDevice);
	EXT_INSTANCE_PROC(vkEnumerateDeviceExtensionProperties);
	EXT_INSTANCE_PROC(vkDestroyDevice);
	EXT_INSTANCE_PROC(vkGetDeviceQueue);
	EXT_INSTANCE_PROC(vkDestroySurfaceKHR);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
	EXT_INSTANCE_PROC(vkCreateSwapchainKHR);
	EXT_INSTANCE_PROC(vkDestroySwapchainKHR);
	EXT_INSTANCE_PROC(vkGetSwapchainImagesKHR);
	EXT_INSTANCE_PROC(vkAcquireNextImageKHR);
	EXT_INSTANCE_PROC(vkQueuePresentKHR);
	EXT_INSTANCE_PROC(vkCreateSemaphore);
	EXT_INSTANCE_PROC(vkDestroySemaphore);
	EXT_INSTANCE_PROC(vkCreateCommandPool);
	EXT_INSTANCE_PROC(vkDestroyCommandPool);
	EXT_INSTANCE_PROC(vkAllocateCommandBuffers);
	EXT_INSTANCE_PROC(vkFreeCommandBuffers);
	EXT_INSTANCE_PROC(vkQueueSubmit);
	EXT_INSTANCE_PROC(vkBeginCommandBuffer);
	EXT_INSTANCE_PROC(vkEndCommandBuffer);
	EXT_INSTANCE_PROC(vkDeviceWaitIdle);
	EXT_INSTANCE_PROC(vkCmdClearColorImage);
	EXT_INSTANCE_PROC(vkCmdPipelineBarrier);
	EXT_INSTANCE_PROC(vkCreateRenderPass);
	EXT_INSTANCE_PROC(vkDestroyRenderPass);
	EXT_INSTANCE_PROC(vkCreateShaderModule);
	EXT_INSTANCE_PROC(vkDestroyShaderModule);
	EXT_INSTANCE_PROC(vkCreateImageView);
	EXT_INSTANCE_PROC(vkDestroyImageView);
	EXT_INSTANCE_PROC(vkCreateFramebuffer);
	EXT_INSTANCE_PROC(vkDestroyFramebuffer);
	EXT_INSTANCE_PROC(vkCreatePipelineLayout);
	EXT_INSTANCE_PROC(vkDestroyPipelineLayout);
	EXT_INSTANCE_PROC(vkCreateGraphicsPipelines);
	EXT_INSTANCE_PROC(vkCreateComputePipelines);
	EXT_INSTANCE_PROC(vkDestroyPipeline);
	EXT_INSTANCE_PROC(vkCmdBeginRenderPass);
	EXT_INSTANCE_PROC(vkCmdBindPipeline);
	EXT_INSTANCE_PROC(vkCmdDraw);
	EXT_INSTANCE_PROC(vkCmdEndRenderPass);
	EXT_INSTANCE_PROC(vkCreateDebugUtilsMessengerEXT);
	EXT_INSTANCE_PROC(vkDestroyDebugUtilsMessengerEXT);
	EXT_INSTANCE_PROC(vkCreateFence);
	EXT_INSTANCE_PROC(vkDestroyFence);
	EXT_INSTANCE_PROC(vkGetFenceStatus);
	EXT_INSTANCE_PROC(vkResetFences);
	EXT_INSTANCE_PROC(vkWaitForFences);
	EXT_INSTANCE_PROC(vkCreateBuffer);
	EXT_INSTANCE_PROC(vkDestroyBuffer);
	EXT_INSTANCE_PROC(vkGetBufferMemoryRequirements);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceMemoryProperties);
	EXT_INSTANCE_PROC(vkAllocateMemory);
	EXT_INSTANCE_PROC(vkFreeMemory);
	EXT_INSTANCE_PROC(vkBindBufferMemory);
	EXT_INSTANCE_PROC(vkMapMemory);
	EXT_INSTANCE_PROC(vkFlushMappedMemoryRanges);
	EXT_INSTANCE_PROC(vkUnmapMemory);
	EXT_INSTANCE_PROC(vkCmdBindVertexBuffers);
	EXT_INSTANCE_PROC(vkCmdCopyBuffer);
	EXT_INSTANCE_PROC(vkCreateImage);
	EXT_INSTANCE_PROC(vkDestroyImage);
	EXT_INSTANCE_PROC(vkGetImageMemoryRequirements);
	EXT_INSTANCE_PROC(vkBindImageMemory);
	EXT_INSTANCE_PROC(vkCmdCopyBufferToImage);
	EXT_INSTANCE_PROC(vkCreateSampler);
	EXT_INSTANCE_PROC(vkDestroySampler);
	EXT_INSTANCE_PROC(vkCreateDescriptorPool);
	EXT_INSTANCE_PROC(vkDestroyDescriptorPool);
	EXT_INSTANCE_PROC(vkCreateDescriptorSetLayout);
	EXT_INSTANCE_PROC(vkDestroyDescriptorSetLayout);
	EXT_INSTANCE_PROC(vkAllocateDescriptorSets);
	EXT_INSTANCE_PROC(vkFreeDescriptorSets);
	EXT_INSTANCE_PROC(vkUpdateDescriptorSets);
	EXT_INSTANCE_PROC(vkCmdBindDescriptorSets);
	EXT_INSTANCE_PROC(vkCmdBindIndexBuffer);
	EXT_INSTANCE_PROC(vkCmdDrawIndexed);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceFormatProperties);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceImageFormatProperties);
	EXT_INSTANCE_PROC(vkCmdCopyImageToBuffer);
	EXT_INSTANCE_PROC(vkSetDebugUtilsObjectNameEXT);
	EXT_INSTANCE_PROC(vkQueueBeginDebugUtilsLabelEXT);
	EXT_INSTANCE_PROC(vkQueueEndDebugUtilsLabelEXT);
	EXT_INSTANCE_PROC(vkCmdBeginDebugUtilsLabelEXT);
	EXT_INSTANCE_PROC(vkCmdEndDebugUtilsLabelEXT);
	EXT_INSTANCE_PROC(vkCreateQueryPool);
	EXT_INSTANCE_PROC(vkDestroyQueryPool);
	EXT_INSTANCE_PROC(vkGetQueryPoolResults);
	EXT_INSTANCE_PROC(vkCmdWriteTimestamp);
	EXT_INSTANCE_PROC(vkCmdSetViewport);
	EXT_INSTANCE_PROC(vkCmdSetScissor);
	EXT_INSTANCE_PROC(vkCmdPushConstants);
	EXT_INSTANCE_PROC(vkCmdSetLineWidth);
	EXT_INSTANCE_PROC(vkCmdNextSubpass);
	EXT_INSTANCE_PROC(vkQueueWaitIdle);
	EXT_INSTANCE_PROC(vkCmdBeginQuery);
	EXT_INSTANCE_PROC(vkCmdEndQuery);
	EXT_INSTANCE_PROC(vkGetDeviceMemoryCommitment);
	EXT_INSTANCE_PROC(vkCmdBlitImage);
	EXT_INSTANCE_PROC(vkCreatePipelineCache);
	EXT_INSTANCE_PROC(vkDestroyPipelineCache);
	EXT_INSTANCE_PROC(vkGetPipelineCacheData);
	EXT_INSTANCE_PROC(vkMergePipelineCaches);
	EXT_INSTANCE_PROC(vkCmdResetQueryPool);
	EXT_INSTANCE_PROC(vkGetPhysicalDeviceMemoryProperties2);
	EXT_INSTANCE_PROC(vkAcquireFullScreenExclusiveModeEXT);
	EXT_INSTANCE_PROC(vkReleaseFullScreenExclusiveModeEXT);
	EXT_INSTANCE_PROC(vkCmdDrawIndirect);
	EXT_INSTANCE_PROC(vkCmdDrawIndexedIndirect);
	EXT_INSTANCE_PROC(vkResetDescriptorPool);
	EXT_INSTANCE_PROC(vkCmdDispatch);

#ifdef _WIN32
	EXT_INSTANCE_PROC(vkCreateWin32SurfaceKHR);
#endif
	
	void vkInit(_In_ InstanceHndl instance, _In_ const vector<const char*> enalbedExtensions);
}