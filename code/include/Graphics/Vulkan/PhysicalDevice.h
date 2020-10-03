#pragma once
#include <tuple>
#include "LogicalDevice.h"

namespace Pu
{
	class VulkanInstance;
	class Surface;
	struct EventArgs;

	/* Defines a Vulkan-compatible physical device. */
	class PhysicalDevice
	{
	public:
		PhysicalDevice(_In_ const PhysicalDevice&) = delete;
		/* Move constructor. */
		PhysicalDevice(_In_ PhysicalDevice &&value);
		/* Releases the physical device. */
		~PhysicalDevice(void);

		_Check_return_ PhysicalDevice& operator =(_In_ const PhysicalDevice&) = delete;
		/* Move assignment. */
		_Check_return_ PhysicalDevice& operator =(_In_ PhysicalDevice &&other);

		/* Gets the queue families available on this physical device. */
		_Check_return_ vector<QueueFamilyProperties> GetQueueFamilies(void) const;
		/* Creates a new logical device from this physical device (requires delete!). */
		_Check_return_ LogicalDevice* CreateLogicalDevice(_In_ const DeviceCreateInfo &createInfo) const;
		/* Gets all extensions supported by a specific layer (UTF-8) or all enabled extensions if the layer is nullptr or this physical device. */
		_Check_return_ vector<ExtensionProperties> GetSupportedExtensions(_In_ const char *layer) const;
		/* Checks whether a specific extension is supported. */
		_Check_return_ bool IsExtensionSupported(_In_ const char *extension) const;
		/* Checks whether specific extensions are supported. */
		_Check_return_ bool AreExtensionsSupported(_In_ std::initializer_list<const char*> extensions) const;
		/* Gets the format properties for the specified format. */
		_Check_return_ FormatProperties GetFormatProperties(_In_ Format format) const;
		/* Gets the alligned size for a specific object in a uniform buffer. */
		_Check_return_ DeviceSize GetUniformBufferOffsetAllignment(_In_ DeviceSize size) const;
		/* Gets the best graphics queue family index for a specific surface. */
		_Check_return_ uint32 GetBestGraphicsQueueFamily(_In_ const Surface &surface) const;
		/* Gets the best graphics queue family index, with no presentation taken into account. */
		_Check_return_ uint32 GetBestGraphicsQueueFamily(void) const;
		/* Gets the best transfer queue family index. */
		_Check_return_ uint32 GetBestTransferQueueFamily(void) const;
		/* Gets the amount of device local bytes supported by the device. */
		_Check_return_ DeviceSize GetDeviceLocalBytes(void) const;
		/* Attempts to get the device local bytes used by the device. */
		_Check_return_ bool TryGetUsedDeviceLocalBytes(_Out_ DeviceSize &result) const;

		/* Gets the maximum supported version of Vulkan supported by the physical device. */
		_Check_return_ inline std::tuple<uint32, uint32, uint32> GetVulkanVersion(void) const
		{
			return std::make_tuple(getMajor(properties.ApiVersion), getMinor(properties.ApiVersion), getPatch(properties.ApiVersion));
		}

		/* Gets the vendor-specified version of the driver. */
		_Check_return_ inline std::tuple<uint32, uint32, uint32> GetDriverVersion(void) const
		{
			return std::make_tuple(getMajor(properties.DriverVersion), getMinor(properties.DriverVersion), getPatch(properties.DriverVersion));
		}

		/* Gets the unique indentifier for the physical device vendor. */
		_Check_return_ inline uint32 GetVendorID(void) const
		{
			return properties.VendorID;
		}

		/* Gets the unique indentifier for the physical device. */
		_Check_return_ inline uint32 GetDeviceID(void) const
		{
			return properties.DeviceID;
		}

		/* Gets the type of the physical device. */
		_Check_return_ inline PhysicalDeviceType GetType(void) const
		{
			return properties.DeviceType;
		}

		/* Gets the UTF-8 name of the physical device. */
		_Check_return_ inline const char* GetName(void) const
		{
			return properties.DeviceName;
		}

		/* Gets the limits of the physical device. */
		_Check_return_ inline const PhysicalDeviceLimits& GetLimits(void) const
		{
			return properties.Limits;
		}

		/* Gets the sparse properties of the physical device. */
		_Check_return_ inline const PhysicalDeviceSparseProperties& GetSparseProperties()
		{
			return properties.SparseProperties;
		}

		/* Gets the capabilities of the physical device.  */
		_Check_return_ inline const PhysicalDeviceFeatures& GetSupportedFeatures(void) const
		{
			return supportedFeatures.Features;
		}

		/* Gets the features that were enabled on the physical device. */
		_Check_return_ inline const PhysicalDeviceFeatures& GetEnabledFeatures(void) const
		{
			return enabledFeatures.Features;
		}

		/* Gets the capabilities of all memory types supported by this physical device. */
		_Check_return_ inline const PhysicalDeviceMemoryProperties& GetMemoryProperties(void) const
		{
			return memory;
		}

		/* Gets the amount of memory allocations created by vkAllocateMemory from this physical device. */
		_Check_return_ inline uint32 GetAllocationsCount(void) const
		{
			return memAllocs;
		}

		/* Gets the amount of sampler allocations created by vkCreateSampler from this physical device. */
		_Check_return_ inline uint32 GetSamplerCount(void) const
		{
			return samplerAllocs;
		}

		/* Gets the default number of invocations per subgroup. */
		_Check_return_ inline uint32 GetSubgroupSize(void) const
		{
			return subgroup.SubgroupSize;
		}

		/* Gets which shader stages support subgroup operations (compute is always supported). */
		_Check_return_ inline ShaderStageFlags GetSubgroupSupportedStages(void) const
		{
			return subgroup.SupportedStages;
		}

		/* Gets which subgroup operations are supported. */
		_Check_return_ inline SubgroupFeatureFlags GetSubgroupFeatures(void) const
		{
			return subgroup.SupportedOperations;
		}

		/* Gets the device specified properties for conservative rasterization. */
		_Check_return_ inline const PhysicalDeviceConservativeRasterizationProperties& GetConservativeRasterizationProperties(void) const
		{
			return conservativeRasterization;
		}

		/* Gets the Vulkan instance of this physical device. */
		_Check_return_ inline const VulkanInstance& GetInstance(void) const
		{
			return *parent;
		}

		/* Gets the Vulkan instance of this physical device. */
		_Check_return_ inline VulkanInstance& GetInstance(void)
		{
			return *parent;
		}

	private:
		friend class VulkanInstance;
		friend class LogicalDevice;
		friend class Surface;
		friend class Buffer;
		friend class Image;
		friend class Sampler;
		friend class Application;
		friend class GameWindow;
		friend class PipelineCache;
		friend class Pipeline;
		friend class GraphicsPipeline;

		VulkanInstance *parent;
		PhysicalDeviceHndl hndl;
		PhysicalDeviceProperties properties;
		PhysicalDeviceSubgroupProperties subgroup;
		PhysicalDeviceConservativeRasterizationProperties conservativeRasterization;
		PhysicalDeviceFeatures2 supportedFeatures, enabledFeatures;
		PhysicalDeviceMemoryProperties memory;
		bool canQueryMemoryUsage, exclusiveFullScreenSupported, executablePropertiesSupported, conservativeRasterizationSupported;
		uint32 memAllocs, samplerAllocs;

		PhysicalDevice(VulkanInstance &parent, PhysicalDeviceHndl hndl);

		ImageFormatProperties GetImageFormatProperties(const ImageCreateInfo &createInfo);
		bool SupportsPlutonium(const Surface &surface) const;
		uint32 GetBestGraphicsQueueFamilyInternal(const Surface *surface) const;
		bool GetBestMemoryType(uint32 memoryTypeBits, MemoryPropertyFlags &requiredProperties, MemoryPropertyFlags optionalProperties, uint32 &index);
		void OnParentDestroyed(const VulkanInstance&, EventArgs);
	};
}