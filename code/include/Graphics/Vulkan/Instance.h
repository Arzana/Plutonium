#pragma once
#include "PhysicalDevice.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines an instance of a Vulkan application. */
	class VulkanInstance final
	{
	public:
		/* Occurs when the Vulkan instance is destroyed. */
		EventBus<const VulkanInstance, EventArgs> OnDestroy;

		/* Initializes a new instance of a Vulkan instance. */
		VulkanInstance(_In_ const char *applicationName, _In_opt_ bool log = LogAvailableVulkanExtensionsAndLayers, _In_opt_ std::initializer_list<const char*> extensions = std::initializer_list<const char*>(), _In_opt_ std::initializer_list<const char*> optionalExtensions = std::initializer_list<const char*>(), _In_opt_ int32 major = 1, _In_opt_ int32 minor = 1, _In_opt_ int32 patch = 0);
		VulkanInstance(_In_ const VulkanInstance&) = delete;
		/* Move constructor. */
		VulkanInstance(_In_ VulkanInstance &&value);
		/* Destroys the instance. */
		~VulkanInstance(void) noexcept
		{
			Destroy();
		}

		_Check_return_ VulkanInstance& operator =(_In_ const VulkanInstance&) = delete;
		/* Move assignment. */
		_Check_return_ VulkanInstance& operator =(_In_ VulkanInstance &&other);

		/* Gets the maximum supported version of Vulkan supported by instance-level functionality. */
		_Check_return_ static std::tuple<uint32, uint32, uint32> GetSupportedVersion(void);
		/* Gets all extensions supported by a specified layer (UTF-8) or all implicity enabled extensions if layer is nullptr. */
		_Check_return_ static vector<ExtensionProperties> GetSupportedExtensions(_In_ const char *layer);
		/* Get all supported layers. */
		_Check_return_ static vector<LayerProperties> GetSupportedLayers(void);
		/* Checks whether a specific extension is supported. */
		_Check_return_ static bool IsExtensionSupported(_In_ const char *extension);
		/* Checks whether a specific layer is supported. */
		_Check_return_ static bool IsLayerSupported(_In_ const char *layer);
		/* Checks whether specific extensions are supported. */
		_Check_return_ static bool AreExtensionsSupported(_In_ std::initializer_list<const char*> extensions);
		/* Checks whether specific layers are supported. */
		_Check_return_ static bool AreLayersSupported(_In_ std::initializer_list<const char*> layers);

		/* Checks whether a specific extension is enabled. */
		_Check_return_ inline bool IsExtensionEnabled(_In_ const char *extension) const
		{
			return enabledExtensions.contains(extension);
		}

		/* Gets the amount of physical devices visible to this Vulkan instance. */
		_Check_return_ inline size_t GetPhysicalDeviceCount(void) const
		{
			return physicalDevices.size();
		}

		/* Gets all the physical devices visible to this Vulkan instance. */
		_Check_return_ inline const vector<PhysicalDevice>& GetPhysicalDevices(void) const
		{
			return physicalDevices;
		}

	private:
		friend class PhysicalDevice;
		friend class LogicalDevice;
		friend class Surface;
		friend class GameWindow;

		static PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
		static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
		static PFN_vkCreateInstance vkCreateInstance;

		InstanceHndl hndl;
		vector<PhysicalDevice> physicalDevices;
		vector<const char*> enabledExtensions;

		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
		PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
		PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
		PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
		PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
		PFN_vkCreateDevice vkCreateDevice;
		PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
		PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
		PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR vkGetPhysicalDeviceSurfaceCapabilities2KHR;

#ifdef _WIN32
		PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif

		static void LoadStaticProcs(void);

		void Destroy(void);
		void LoadInstanceProcs(void);
		void QueryPhysicalDevices(void);
		void LogAvailableExtensionsAndLayers(void) const;
		
#if defined (_DEBUG) || defined(VULKAN_FORCE_VALIDATION)
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
		PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
		PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabelEXT;
		PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabelEXT;
		PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
		PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
		DebugUtilsMessengerHndl msgHndl;

		static VKAPI_ATTR Bool32 VKAPI_CALL DebugCallback(DebugUtilsMessageSeverityFlag severity, DebugUtilsMessageTypeFlag, const DebugUtilsMessengerCallbackData *data, void*);
		
		void SetUpDebugLayer(void);
#endif
	};
}