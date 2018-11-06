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
		VulkanInstance(_In_ const char *applicationName, _In_opt_ int32 major = 0, _In_opt_ int32 minor = 0, _In_opt_ int32 patch = 0);
		VulkanInstance(_In_ const VulkanInstance&) = delete;
		VulkanInstance(_In_ VulkanInstance &&value);
		/* Destroys the instance. */
		~VulkanInstance(void) noexcept
		{
			Destroy();
		}

		_Check_return_ VulkanInstance& operator =(_In_ const VulkanInstance&) = delete;
		_Check_return_ VulkanInstance& operator =(_In_ VulkanInstance &&other);

		/* Gets the maximum supported version of Vulkan supported by instance-level functionality. */
		_Check_return_ static std::tuple<uint32, uint32, uint32> GetSupportedVersion(void);
		/* Gets all extensions supported by a specified layer (UTF-8) or all implicity enabled extensions if layer is nullptr. */
		_Check_return_ static vector<ExtensionProperties> GetSupportedExtensions(_In_ const char *layer);
		/* Get all supported layers. */
		_Check_return_ static vector<LayerProperties> GetSupportedLayers(void);

		/* Gets all Vulkan-compatible physical devices. */
		_Check_return_ vector<PhysicalDevice> GetPhysicalDevices(void) const;
		/* Gets the handle of this instance. */
		_Check_return_ inline InstanceHndl GetHandle(void) const
		{
			return hndl;
		}

	private:
		friend class PhysicalDevice;

		static PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
		static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
		static PFN_vkCreateInstance vkCreateInstance;

		InstanceHndl hndl;
		PFN_vkDestroyInstance vkDestroyInstance;
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
		PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;

		static void LoadStaticProcs(void);

		void Destroy(void);
		void LoadInstanceProcs(void);
	};
}