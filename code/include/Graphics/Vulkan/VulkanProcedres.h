#pragma once
#include "VulkanObjects.h"

namespace Pu
{
	using PFN_vkEnumerateInstanceVersion = _Check_return_ VkApiResult(VKAPI_PTR)(_Out_ uint32 *apiVersion);
	using PFN_vkEnumerateInstanceExtensionProperties = _Check_return_ VkApiResult(VKAPI_PTR)(_In_opt_ const char *layerName, _Inout_ uint32 *propertyCount, _Out_ ExtensionProperties *properties);
	using PFN_vkEnumerateInstanceLayerProperties = _Check_return_ VkApiResult(VKAPI_PTR)(_Inout_ uint32_t *propertyCount, _Out_ LayerProperties *properties);
	using PFN_vkEnumeratePhysicalDevices = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ InstanceHndl instance, _Inout_ uint32 *physicalDeviceCount, _Out_ PhysicalDeviceHndl *physicalDevices);
	using PFN_vkCreateInstance = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ const InstanceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ InstanceHndl *instance);
	using PFN_vkDestroyInstance = void(VKAPI_PTR)(_In_ InstanceHndl instance, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetPhysicalDeviceProperties = void(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _Out_ PhysicalDeviceProperties *properties);
	using PFN_vkGetPhysicalDeviceFeatures = void(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _Out_ PhysicalDeviceFeatures *features);
	using PFN_vkGetPhysicalDeviceQueueFamilyProperties = void(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _Inout_ uint32 *queueFamilyPropertyCount, _Out_ QueueFamilyProperties *queueFamilyProperties);
	using PFN_vkCreateDevice = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_ const DeviceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ DeviceHndl *device);
	using PFN_vkEnumerateDeviceExtensionProperties = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_opt_ const char *layerName, _Inout_ uint32 *propertyCount, _Out_ ExtensionProperties *properties);
	using PFN_vkDestroyDevice = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetDeviceQueue = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ uint32 queueFamilyIndex, _In_ uint32 queueIndex, _Out_ QueueHndl *queue);
	using PFN_vkDestroySurfaceKHR = void(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ SurfaceHndl surface, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetPhysicalDeviceSurfaceSupportKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_ uint32 queueFamilyIndex, _In_ SurfaceHndl surface, _Out_ Bool32 *supported);
	using PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_ SurfaceHndl surface, _Out_ SurfaceCapabilities *surfaceCapabilities);
	using PFN_vkGetPhysicalDeviceSurfaceFormatsKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_ SurfaceHndl surface, _Inout_ uint32 *surfaceFormatCount, _Out_ SurfaceFormat *surfaceFormats);
	using PFN_vkGetPhysicalDeviceSurfacePresentModesKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _In_ SurfaceHndl surface, _Inout_ uint32 *presentModeCount, _Out_ PresentMode *presentModes);
	using PFN_vkCreateSwapchainKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const SwapchainCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ SwapchainHndl *swapChain);
	using PFN_vkDestroySwapchainKHR = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ SwapchainHndl swapchain, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetSwapchainImagesKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ SwapchainHndl swapchain, _Inout_ uint32 *swapchainImageCount, _Out_ ImageHndl *swapchainImages);
	using PFN_vkAcquireNextImageKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ SwapchainHndl swapchain, _In_ uint64 timeout, _In_opt_ SemaphoreHndl semaphore, _In_opt_ FenceHndl fence, _Out_ uint32 *imageIndex);
	using PFN_vkQueuePresentKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ QueueHndl queue, _In_ const PresentInfo *presentInfo);
	using PFN_vkCreateSemaphore = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const SemaphoreCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ SemaphoreHndl *semaphore);
	using PFN_vkDestroySemaphore = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ SemaphoreHndl semaphore, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateCommandPool = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const CommandPoolCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ CommandPoolHndl *commandPool);
	using PFN_vkDestroyCommandPool = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ CommandPoolHndl commandPool, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkAllocateCommandBuffers = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, const CommandBufferAllocateInfo *allocateInfo, _Out_ CommandBufferHndl *commandBuffers);
	using PFN_vkFreeCommandBuffers = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ CommandPoolHndl commandPool, _In_ uint32 commandBufferCount, _In_ const CommandBufferHndl *commandBuffers);
	using PFN_vkQueueSubmit = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ QueueHndl queue, _In_ uint32 submitCount, _In_ const SubmitInfo *submits, _In_opt_ FenceHndl fence);
	using PFN_vkBeginCommandBuffer = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ const CommandBufferBeginInfo *beginInfo);
	using PFN_vkEndCommandBuffer = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer);
	using PFN_vkDeviceWaitIdle = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device);
	using PFN_vkCmdClearColorImage = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ ImageHndl image, _In_ ImageLayout imageLayout, _In_ ClearColorValue color, uint32 rangeCount, const ImageSubresourceRange *ranges);
	using PFN_vkCmdPipelineBarrier = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ PipelineStageFlag srcStageMask, _In_ PipelineStageFlag dstStageMask, _In_ DependencyFlag dependencyFlags, _In_ uint32 memoryBarrierCount, _In_opt_ const MemoryBarrier *memoryBarriers, _In_ uint32 bufferMemoryBarrierCount, _In_opt_ BufferMemoryBarrier *bufferMemoryBarriers, _In_ uint32 imageMemoryBarrierCount, _In_opt_ const ImageMemoryBarrier *imageMemoryBarriers);
	using PFN_vkCreateRenderPass = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const RenderPassCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ RenderPassHndl *renderPass);
	using PFN_vkDestroyRenderPass = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ RenderPassHndl renderPass, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateShaderModule = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const ShaderModuleCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ ShaderModuleHndl *shaderModule);
	using PFN_vkDestroyShaderModule = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ ShaderModuleHndl shaderModule, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateImageView = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const ImageViewCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ ImageViewHndl *view);
	using PFN_vkDestroyImageView = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ ImageViewHndl view, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateFramebuffer = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, const FramebufferCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ FramebufferHndl *framebuffer);
	using PFN_vkDestroyFramebuffer = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ FramebufferHndl framebuffer, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreatePipelineLayout = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const PipelineLayoutCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ PipelineLayoutHndl *pipelineLayout);
	using PFN_vkDestroyPipelineLayout = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ PipelineLayoutHndl pipelineLayout, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateGraphicsPipelines = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_opt_ PipelineCacheHndl pipelineCache, _In_ uint32 createInfoCount, _In_ const GraphicsPipelineCreateInfo *createInfos, _In_opt_ const AllocationCallbacks *allocator, _Out_ PipelineHndl *pipelines);
	using PFN_vkDestroyPipeline = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ PipelineHndl pipeline, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCmdBeginRenderPass = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ const RenderPassBeginInfo *renderPassBegin, _In_ SubpassContents contents);
	using PFN_vkCmdBindPipeline = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ PipelineBindPoint pipelineBindPoint, _In_ PipelineHndl pipeline);
	using PFN_vkCmdDraw = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ uint32 vertexCount, _In_ uint32 instanceCount, _In_ uint32 firstVertex, _In_ uint32 firstInstance);
	using PFN_vkCmdEndRenderPass = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer);
	using PFN_vkCreateDebugUtilsMessengerEXT = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ const DebugUtilsMessengerCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ DebugUtilsMessengerHndl *messenger);
	using PFN_vkDestroyDebugUtilsMessengerEXT = void(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ DebugUtilsMessengerHndl messenger, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkCreateFence = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const FenceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ FenceHndl *fence);
	using PFN_vkDestroyFence = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ FenceHndl fence, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetFenceStatus = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ FenceHndl fence);
	using PFN_vkResetFences = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ uint32 fenceCount, _In_ const FenceHndl *fences);
	using PFN_vkWaitForFences = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ uint32 fenceCount, _In_ const FenceHndl *fences, _In_ Bool32 waitAll, _In_ uint64 timeout);
	using PFN_vkCreateBuffer = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const BufferCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ BufferHndl *buffer);
	using PFN_vkDestroyBuffer = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ BufferHndl buffer, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkGetBufferMemoryRequirements = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ BufferHndl buffer, _Out_ MemoryRequirements *memoryRequirements);
	using PFN_vkGetPhysicalDeviceMemoryProperties = void(VKAPI_PTR)(_In_ PhysicalDeviceHndl physicalDevice, _Out_ PhysicalDeviceMemoryProperties *memoryProperties);
	using PFN_vkAllocateMemory = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ const MemoryAllocateInfo *allocateInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ DeviceMemoryHndl *memory);
	using PFN_vkFreeMemory = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ DeviceMemoryHndl memory, _In_opt_ const AllocationCallbacks *allocator);
	using PFN_vkBindBufferMemory = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ BufferHndl buffer, _In_ DeviceMemoryHndl memory, _In_ DeviceSize memoryOffset);
	using PFN_vkMapMemory = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ DeviceMemoryHndl memory, _In_ DeviceSize offset, _In_ DeviceSize size, _In_ Flags flags, _Out_ void **data);
	using PFN_vkFlushMappedMemoryRanges = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ DeviceHndl device, _In_ uint32 memoryRangeCount, _In_ const MappedMemoryRange *memoryRanges);
	using PFN_vkUnmapMemory = void(VKAPI_PTR)(_In_ DeviceHndl device, _In_ DeviceMemoryHndl memory);
	using PFN_vkCmdBindVertexBuffers = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ uint32 firstBinding, _In_ uint32 bindingCount, _In_ const BufferHndl *buffers, _In_ const DeviceSize *offsets);

#ifdef _WIN32
	using PFN_vkCreateWin32SurfaceKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ const Win32SurfaceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ SurfaceHndl *surface);
#endif
}