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
		_Check_return_ LogicalDevice* CreateLogicalDevice(_In_ const DeviceCreateInfo *createInfo) const;
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

		/* Gets the capabilities of the physical device.  */
		_Check_return_ inline const PhysicalDeviceFeatures& GetFeatures(void) const
		{
			return features;
		}

		/* Gets whether all single-sample 2D sparse resources use standard image block shapes. */
		_Check_return_ inline bool IsStandard2DBlockShape(void) const
		{
			return properties.SparseProperties.ResidencyStandard2DBlockShape;
		}

		/* Gets whether all multisample 2D sparse resources use standard image block shapes. */
		_Check_return_ inline bool IsStandard2DMutisampleBlockShape(void) const
		{
			return properties.SparseProperties.ResidencyStandard2DMultisampleBlockShape;
		}

		/* Gets whether all 3D sparse resources use standard image block shapes. */
		_Check_return_ inline bool IsStandard3DBlockShape(void) const
		{
			return properties.SparseProperties.ResidencyStandard3DBlockShape;
		}

		/* Gets whether images with mip level dimensions that are not integer multiples of the corresponding dimensions of the sparse image block may be places in the mip tail. */
		_Check_return_ inline bool IsAlignedMipSize(void) const
		{
			return properties.SparseProperties.ResidencyAlignedMipSize;
		}

		/* Specifies whether the physical device can consistently access non-resident regions of a resource. */
		_Check_return_ inline bool IsNonResidentStrict(void) const
		{
			return properties.SparseProperties.ResidencyNonResidentStrict;
		}

	private:
		friend class VulkanInstance;
		friend class LogicalDevice;
		friend class Surface;
		friend class Buffer;
		friend class Image;
		friend class Application;

		VulkanInstance &parent;
		PhysicalDeviceHndl hndl;
		PhysicalDeviceProperties properties;
		PhysicalDeviceFeatures features;
		PhysicalDeviceMemoryProperties memory;

		PhysicalDevice(VulkanInstance &parent, PhysicalDeviceHndl hndl);

		ImageFormatProperties GetImageFormatProperties(const ImageCreateInfo &createInfo);
		bool SupportsPlutonium(const Surface &surface) const;
		uint32 GetBestGraphicsQueueFamily(const Surface &surface) const;
		uint32 GetBestTransferQueueFamily(void) const;
		bool GetBestMemoryType(uint32 memoryTypeBits, MemoryPropertyFlag &memoryProperties, bool preferCaching, uint32 &index);
		void OnParentDestroyed(const VulkanInstance&, EventArgs);
	};
}