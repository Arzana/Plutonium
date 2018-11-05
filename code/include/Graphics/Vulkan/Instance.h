#pragma once
#include "Loader.h"
#include "VulkanObjects.h"
#include "Core/Collections/Vector.h"

namespace Pu
{
	/* Defines an instance of a Vulkan application. */
	class VulkanInstance final
	{
	public:
		/* Initializes a new instance of a Vulkan instance. */
		VulkanInstance(_In_ const char *applicationName, _In_opt_ int32 major = 0, _In_opt_ int32 minor = 0, _In_opt_ int32 patch = 0);
		VulkanInstance(_In_ const VulkanInstance&) = delete;
		/* Move constructor. */
		VulkanInstance(_In_ VulkanInstance &&value);
		/* Destroys the instance. */
		~VulkanInstance(void) noexcept
		{
			Destroy();
		}

		_Check_return_ VulkanInstance& operator =(_In_ const VulkanInstance&) = delete;
		/* Moves the value of other into the vulkan instance. */
		_Check_return_ VulkanInstance& operator =(_In_ VulkanInstance &&other);

		/* Gets the maximum supported version of Vulkan supported by instance-level functionality. */
		_Check_return_ static void GetSupportedVersion(_Out_ uint32 &major, _Out_ uint32 &minor, _Out_ uint32 &patch);
		/* Gets all extensions supported by a specified layer (UTF-8) or all implicity enabled extensions if layer is nullptr. */
		_Check_return_ static vector<ExtensionProperties> GetSupportedExtensions(_In_ const char *layer);
		/* Get all supported layers. */
		_Check_return_ static vector<LayerProperties> GetSupportedLayers(void);

	private:
		using PFN_vkEnumerateInstanceVersion = Result(VKAPI_PTR)(uint32 *apiVersion);
		using PFN_vkEnumerateInstanceExtensionProperties = Result(VKAPI_PTR)(const char *layerName, uint32 *propertyCount, ExtensionProperties *properties);
		using PFN_vkEnumerateInstanceLayerProperties = Result(VKAPI_PTR)(uint32_t *propertyCount, LayerProperties *properties);
		using PFN_vkCreateInstance = Result(VKAPI_PTR)(const InstanceCreateInfo *createInfo, const AllocationCallbacks *allocator, InstanceHndl *instance);
		using PFN_vkDestroyInstance = void(VKAPI_PTR)(InstanceHndl instance, const AllocationCallbacks *allocator);

		static PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
		static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
		static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
		static PFN_vkCreateInstance vkCreateInstance;

		InstanceHndl hndl;
		PFN_vkDestroyInstance vkDestroyInstance;

		static void LoadStaticProcs(void);

		void Destroy(void);
		void LoadInstanceProcs(void);
	};
}