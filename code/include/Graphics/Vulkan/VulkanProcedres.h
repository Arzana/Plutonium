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

#ifdef _WIN32
	using PFN_vkCreateWin32SurfaceKHR = _Check_return_ VkApiResult(VKAPI_PTR)(_In_ InstanceHndl instance, _In_ const Win32SurfaceCreateInfo *createInfo, _In_opt_ const AllocationCallbacks *allocator, _Out_ SurfaceHndl *surface);
#endif
}