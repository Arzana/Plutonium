#include "Graphics/Vulkan/PhysicalDevice.h"
#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Surface.h"

using namespace Pu;

Pu::PhysicalDevice::PhysicalDevice(PhysicalDevice && value)
	: parent(value.parent), hndl(value.hndl), properties(std::move(value.properties)),
	supportedFeatures(std::move(value.supportedFeatures)), memory(std::move(value.memory)),
	enabledFeatures(std::move(value.enabledFeatures)), canQueryMemoryUsage(value.canQueryMemoryUsage),
	exclusiveFullScreenSupported(value.exclusiveFullScreenSupported),
	executablePropertiesSupported(value.executablePropertiesSupported),
	memAllocs(value.memAllocs), samplerAllocs(value.samplerAllocs),
	conservativeRasterization(std::move(value.conservativeRasterization)),
	conservativeRasterizationSupported(value.conservativeRasterizationSupported),
	lineFeatures(std::move(value.lineFeatures)), lineSubPixelPrecisionBits(value.lineSubPixelPrecisionBits),
	fancyLineRasterizationSupported(value.fancyLineRasterizationSupported)
{
	value.hndl = nullptr;
}

Pu::PhysicalDevice::~PhysicalDevice(void)
{
	parent->OnDestroy.Remove(*this, &PhysicalDevice::OnParentDestroyed);
}

PhysicalDevice & Pu::PhysicalDevice::operator=(PhysicalDevice && other)
{
	if (this != &other)
	{
		parent->OnDestroy.Remove(*this, &PhysicalDevice::OnParentDestroyed);

		parent = other.parent;
		hndl = other.hndl;
		properties = std::move(other.properties);
		supportedFeatures = std::move(other.supportedFeatures);
		enabledFeatures = std::move(other.enabledFeatures);
		conservativeRasterization = std::move(other.conservativeRasterization);
		lineFeatures = std::move(other.lineFeatures);
		memory = std::move(other.memory);
		canQueryMemoryUsage = other.canQueryMemoryUsage;
		exclusiveFullScreenSupported = other.exclusiveFullScreenSupported;
		executablePropertiesSupported = other.executablePropertiesSupported;
		conservativeRasterizationSupported = other.conservativeRasterizationSupported;
		fancyLineRasterizationSupported = other.fancyLineRasterizationSupported;
		memAllocs = other.memAllocs;
		samplerAllocs = other.samplerAllocs;
		lineSubPixelPrecisionBits = other.lineSubPixelPrecisionBits;

		parent->OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);
		other.hndl = nullptr;
	}

	return *this;
}

vector<QueueFamilyProperties> Pu::PhysicalDevice::GetQueueFamilies(void) const
{
	/* Query the amount of queue families defined. */
	uint32 count;
	parent->vkGetPhysicalDeviceQueueFamilyProperties(hndl, &count, nullptr);

	/* Early out if the count is less than one. */
	if (count < 1) return vector<QueueFamilyProperties>();

	/* Query all queue familie properties. */
	vector<QueueFamilyProperties> result(count);
	parent->vkGetPhysicalDeviceQueueFamilyProperties(hndl, &count, result.data());

	return result;
}

LogicalDevice * Pu::PhysicalDevice::CreateLogicalDevice(const DeviceCreateInfo & createInfo) const
{
	/* Create new logical device. */
	DeviceHndl device;
	VK_VALIDATE(parent->vkCreateDevice(hndl, &createInfo, nullptr, &device), PFN_vkCreateDevice);

	/* Log creation. */
	const auto[major, minor, patch] = GetVulkanVersion();
	Log::Message("Created new Vulkan v%u.%u.%u logical device on %s", major, minor, patch, GetName());

	return new LogicalDevice(const_cast<PhysicalDevice&>(*this), device, createInfo);
}

vector<ExtensionProperties> Pu::PhysicalDevice::GetSupportedExtensions(const char * layer) const
{
	/* Query the amount of properties defined. */
	uint32 count;
	parent->vkEnumerateDeviceExtensionProperties(hndl, layer, &count, nullptr);

	/* Realy out if the count is less than one. */
	if (count < 1) return vector<ExtensionProperties>();

	/* Query all extension properties. */
	vector<ExtensionProperties> result(count);
	parent->vkEnumerateDeviceExtensionProperties(hndl, layer, &count, result.data());

	return result;
}

bool Pu::PhysicalDevice::IsExtensionSupported(const char * extension) const
{
	const vector<ExtensionProperties> props = GetSupportedExtensions(nullptr);
	return props.contains([extension](const ExtensionProperties &prop)
	{
		return !strcmp(prop.ExtensionName, extension);
	});
}

bool Pu::PhysicalDevice::AreExtensionsSupported(std::initializer_list<const char*> extensions) const
{
	const vector<ExtensionProperties> props = GetSupportedExtensions(nullptr);

	size_t found = 0;
	for (const char *extension : extensions)
	{
		for (const ExtensionProperties &prop : props)
		{
			if (!strcmp(prop.ExtensionName, extension))
			{
				++found;
				break;
			}
		}
	}

	return found >= extensions.size();
}

FormatProperties Pu::PhysicalDevice::GetFormatProperties(Format format) const
{
	FormatProperties result;
	parent->vkGetPhysicalDeviceFormatProperties(hndl, format, &result);
	return result;
}

DeviceSize Pu::PhysicalDevice::GetUniformBufferOffsetAllignment(DeviceSize size) const
{
	/* If the minimum allignment is zero we can ignore it. */
	if (properties.Limits.MinUniformBufferOffsetAlignment == 0) return size;
	return (size + properties.Limits.MinUniformBufferOffsetAlignment - 1) & ~(properties.Limits.MinUniformBufferOffsetAlignment - 1);
}

Pu::PhysicalDevice::PhysicalDevice(VulkanInstance & parent, PhysicalDeviceHndl hndl)
	: hndl(hndl), parent(&parent), memAllocs(0), samplerAllocs(0)
{
	PhysicalDeviceProperties2 prop2{ &subgroup };

#pragma warning (push)
#pragma warning (disable:4706)
	PhysicalDevicePipelineExecutablePropertiesFeatures executableProperties;
	PhysicalDeviceLineRasterizationProperties lineProperties;
	if (IsExtensionSupported(u8"VK_KHR_pipeline_executable_properties")) VkPushChain(supportedFeatures.Next, &executableProperties);
	if (conservativeRasterizationSupported = IsExtensionSupported(u8"VK_EXT_conservative_rasterization")) VkPushChain(prop2.Next, &conservativeRasterization);
	if (fancyLineRasterizationSupported = IsExtensionSupported(u8"VK_EXT_line_rasterization"))
	{
		VkPushChain(prop2.Next, &lineProperties);
		VkPushChain(supportedFeatures.Next, &lineFeatures);
	}
#pragma warning (pop)

	/* On destroy check and query the properties for fast access later. */
	parent.OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);
	parent.vkGetPhysicalDeviceProperties2(hndl, &prop2);
	parent.vkGetPhysicalDeviceFeatures2(hndl, &supportedFeatures);
	parent.vkGetPhysicalDeviceMemoryProperties(hndl, &memory);

	properties = prop2.Properties;
	lineSubPixelPrecisionBits = lineProperties.LineSubPixelPrecisionBits;

	/* Querying whether the extensions are supported is slow, so just query it on creation. */
	canQueryMemoryUsage = IsExtensionSupported(u8"VK_EXT_memory_budget");
	exclusiveFullScreenSupported = IsExtensionSupported(u8"VK_EXT_full_screen_exclusive");
	executablePropertiesSupported = executableProperties.PipelineExecutableInfo;
}

ImageFormatProperties Pu::PhysicalDevice::GetImageFormatProperties(const ImageCreateInfo & createInfo)
{
	ImageFormatProperties result;
	parent->vkGetPhysicalDeviceImageFormatProperties(hndl, createInfo.Format, createInfo.ImageType, createInfo.Tiling, createInfo.Usage, createInfo.Flags, &result);
	return result;
}

bool Pu::PhysicalDevice::SupportsPlutonium(const Surface & surface) const
{
	/* The physical device to support the swapchain extension. */
	if (!IsExtensionSupported(u8"VK_KHR_swapchain")) return false;

	/* The physical device need to support at least one presentable graphics queue and one transfer queue. */
	bool graphics = false, transfer = false;
	uint32 i = 0;
	for (const QueueFamilyProperties &family : GetQueueFamilies())
	{
		if (_CrtEnumCheckFlag(family.Flags, QueueFlags::Graphics) && surface.QueueFamilySupportsPresenting(i++, *this)) graphics = true;
		if (_CrtEnumCheckFlag(family.Flags, QueueFlags::Transfer)) transfer = true;
	}

	if (!(graphics && transfer)) return false;

	/* Physical device passed all tests so return true. */
	return true;
}

uint32 Pu::PhysicalDevice::GetBestGraphicsQueueFamilyInternal(const Surface * surface) const
{
	const vector<QueueFamilyProperties> families = GetQueueFamilies();
	uint32 choosen = maxv<uint32>();
	int32 highscore = -1, score = 0;

	/* Loop through all families to find best one. */
	for (uint32 i = 0; i < families.size(); i++, score = 0)
	{
		/* Check if queue supported graphics operations. */
		const QueueFamilyProperties &cur = families[i];
		if (_CrtEnumCheckFlag(cur.Flags, QueueFlags::Graphics))
		{
			/* Check if the queue family supports presenting if a surface is specified. */
			if ((surface && surface->QueueFamilySupportsPresenting(i, *this)) || !surface)
			{
				if ((cur.Flags & QueueFlags::TypeMask) == QueueFlags::Graphics) score += 1;
			}
		}

		/* Update choosen graphics queue if needed. */
		if (score > highscore)
		{
			highscore = score;
			choosen = i;
		}
	}

	/* This should never occur. */
	if (choosen == maxv<uint32>()) Log::Fatal("Unable to find any graphics queue!");

	/* Return the choosen graphics queue or the first queue if no queue's were acceptable. */
	return choosen;
}

uint32 Pu::PhysicalDevice::GetBestGraphicsQueueFamily(const Surface & surface) const
{
	return GetBestGraphicsQueueFamilyInternal(&surface);
}

uint32 Pu::PhysicalDevice::GetBestGraphicsQueueFamily(void) const
{
	return GetBestGraphicsQueueFamilyInternal(nullptr);
}

uint32 Pu::PhysicalDevice::GetBestComputeQueuFamily(void) const
{
	const vector<QueueFamilyProperties> families = GetQueueFamilies();
	uint32 choosen = maxv<uint32>();
	int32 highscore = -1, score = 0;

	/* Loop through all families to find best one. */
	for (uint32 i = 0; i < families.size(); i++, score = 0)
	{
		/* Check if queue supports compute operations. */
		const QueueFamilyProperties &cur = families[i];
		if (_CrtEnumCheckFlag(cur.Flags, QueueFlags::Compute))
		{
			if ((cur.Flags & QueueFlags::TypeMask) == QueueFlags::Compute) score += 1;
		}

		/* Update choosen graphics queue if needed. */
		if (score > highscore)
		{
			highscore = score;
			choosen = i;
		}
	}

	/* This should never occur. */
	if (choosen == maxv<uint32>()) Log::Fatal("Unable to find any compute queue!");

	/* Return the choosen transfer queue. */
	return choosen;
}

uint32 Pu::PhysicalDevice::GetBestTransferQueueFamily(void) const
{
	const vector<QueueFamilyProperties> families = GetQueueFamilies();
	uint32 choosen = maxv<uint32>();
	int32 highscore = -1, score = 0;

	/* Loop through all families to find best one. */
	for (uint32 i = 0; i < families.size(); i++, score = 0)
	{
		/* Check if queue supports transfer operations. */
		const QueueFamilyProperties &cur = families[i];
		if (_CrtEnumCheckFlag(cur.Flags, QueueFlags::Transfer))
		{
			if ((cur.Flags & QueueFlags::TypeMask) == QueueFlags::Transfer) score += 1;
		}

		/* Update choosen graphics queue if needed. */
		if (score > highscore)
		{
			highscore = score;
			choosen = i;
		}
	}

	/* This should never occur. */
	if (choosen == maxv<uint32>()) Log::Fatal("Unable to find any transfer queue!");

	/* Return the choosen transfer queue. */
	return choosen;
}

DeviceSize Pu::PhysicalDevice::GetDeviceLocalBytes(void) const
{
	DeviceSize result = 0;

	for (const MemoryHeap *cur = memory.MemoryHeaps; cur < memory.MemoryHeaps + memory.MemoryHeapCount; cur++)
	{
		if (_CrtEnumCheckFlag(cur->Flags, MemoryHeapFlags::DeviceLocal)) result += cur->Size;
	}

	return result;
}

bool Pu::PhysicalDevice::TryGetUsedDeviceLocalBytes(DeviceSize & result) const
{
	/* Early out if this action isn't possible on this GPU. */
	if (!canQueryMemoryUsage) return false;
	result = 0;

	/* Query the memory budget properties from the physical device. */
	PhysicalDeviceMemoryBudgetProperties budget;
	PhysicalDeviceMemoryProperties2 properties2{ &budget };
	parent->vkGetPhysicalDeviceMemoryProperties2(hndl, &properties2);

	/* Add the used memory for each of the device local flagged heaps. */
	for (uint32 i = 0; i < properties2.MemoryProperties.MemoryHeapCount; i++)
	{
		const MemoryHeap &cur = properties2.MemoryProperties.MemoryHeaps[i];
		if (_CrtEnumCheckFlag(cur.Flags, MemoryHeapFlags::DeviceLocal)) result += budget.HeapUsage[i];
	}

	return true;
}

bool Pu::PhysicalDevice::GetBestMemoryType(uint32 memoryTypeBits, MemoryPropertyFlags &memoryProperties, MemoryPropertyFlags optionalProperties, uint32 & index)
{
	index = maxv<uint32>();
	int32 highscore = -1, score = 0;
	MemoryPropertyFlags checkProperties = memoryProperties;

	/* Loop throug to find all possible memory types. */
	for (uint32 i = 0; i < memory.MemoryTypeCount; ++i, score = 0)
	{
		const MemoryPropertyFlags flags = memory.MemoryTypes[i].PropertyFlags;

		/* Check if type is supported and if the required properties are supported. */
		if ((memoryTypeBits & (1 << i)) && _CrtEnumCheckFlag(flags, checkProperties))
		{
			/* See Scoring document for scoring information. */
			if (_CrtEnumCheckFlag(flags, MemoryPropertyFlags::DeviceLocal)) score += 3;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlags::HostCoherent)) score += 2;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlags::HostVisible)) ++score;
			if (_CrtEnumCheckFlag(flags, optionalProperties)) ++score;

			if (score > highscore)
			{
				highscore = score;
				index = i;
				memoryProperties = flags;
			}
		}
	}

	/* Return whether at least one type was found that supports the  */
	return index != maxv<uint32>();
}

void Pu::PhysicalDevice::OnParentDestroyed(const VulkanInstance &, EventArgs)
{
	Log::Warning("Vulkan instance is deleted before physical device, this caused the physical device to be invalid!");
	hndl = nullptr;
}