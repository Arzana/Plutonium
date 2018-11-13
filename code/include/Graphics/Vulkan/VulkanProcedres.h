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
	using PFN_vkCmdClearColorImage = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ ImageHndl image, _In_ ImageLayout imageLayout, _In_ ClearColorValue color, uint32 rangeCount, const ImageSubresourceRange *ranges);
	using PFN_vkCmdPipelineBarrier = void(VKAPI_PTR)(_In_ CommandBufferHndl commandBuffer, _In_ PipelineStageFlag srcStageMask, _In_ PipelineStageFlag dstStageMask, _In_ DependencyFlag dependencyFlags, _In_ uint32 memoryBarrierCount, _In_opt_ const MemoryBarrier *memoryBarriers, _In_ uint32 bufferMemoryBarrierCount, _In_opt_ BufferMemoryBarrier *bufferMemoryBarriers, _In_ uint32 imageMemoryBarrierCount, _In_opt_ const ImageMemoryBarrier *imageMemoryBarriers);

#ifdef _WIN32
	using PFN_vkCreateWin32SurfaceKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ const Win32SurfaceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ SurfaceHndl *surface);
#endif
}