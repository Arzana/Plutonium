#include "Graphics/Vulkan/PhysicalDevice.h"
#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Surface.h"

using namespace Pu;

Pu::PhysicalDevice::PhysicalDevice(PhysicalDevice && value)
	: PhysicalDevice(*value.parent, value.hndl)
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

		hndl = other.hndl;
		parent = other.parent;
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

LogicalDevice * Pu::PhysicalDevice::CreateLogicalDevice(const DeviceCreateInfo * createInfo) const
{
	/* Create new logical device. */
	DeviceHndl device;
	VK_VALIDATE(parent->vkCreateDevice(hndl, createInfo, nullptr, &device), PFN_vkCreateDevice);

	/* Log creation. */
	const auto[major, minor, patch] = GetVulkanVersion();
	Log::Message("Created new Vulkan v%u.%u.%u logical device on %s", major, minor, patch, GetName());

	return new LogicalDevice(const_cast<PhysicalDevice&>(*this), device, createInfo->QueueCreateInfoCount, createInfo->QueueCreateInfos);
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
	: hndl(hndl), parent(&parent)
{
	/* On destroy check and query the properties for fast access later. */
	parent.OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);
	parent.vkGetPhysicalDeviceProperties(hndl, &properties);
	parent.vkGetPhysicalDeviceFeatures(hndl, &features);
	parent.vkGetPhysicalDeviceMemoryProperties(hndl, &memory);
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
		if (_CrtEnumCheckFlag(family.Flags, QueueFlag::Graphics) && surface.QueueFamilySupportsPresenting(i++, *this)) graphics = true;
		if (_CrtEnumCheckFlag(family.Flags, QueueFlag::Transfer)) transfer = true;
	}

	if (!(graphics && transfer)) return false;

	/* Physical device passed all tests so return true. */
	return true;
}

uint32 Pu::PhysicalDevice::GetBestGraphicsQueueFamily(const Surface & surface) const
{
	const vector<QueueFamilyProperties> families = GetQueueFamilies();
	uint32 choosen = maxv<uint32>();
	int32 highscore = -1, score = 0;

	/* Loop through all families to find best one. */
	for (uint32 i = 0; i < families.size(); i++, score = 0)
	{
		/* Check if queue supported graphics operations and presenting to our surface. */
		const QueueFamilyProperties &cur = families[i];
		if (_CrtEnumCheckFlag(cur.Flags, QueueFlag::Graphics) && surface.QueueFamilySupportsPresenting(i, *this))
		{
			if ((cur.Flags & QueueFlag::TypeMask) == QueueFlag::Graphics) score += 1;
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
		if (_CrtEnumCheckFlag(cur.Flags, QueueFlag::Transfer))
		{
			if ((cur.Flags & QueueFlag::TypeMask) == QueueFlag::Transfer) score += 1;
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

bool Pu::PhysicalDevice::GetBestMemoryType(uint32 memoryTypeBits, MemoryPropertyFlag &memoryProperties, bool preferCaching, uint32 & index)
{
	index = maxv<uint32>();
	int32 highscore = -1, score = 0;
	MemoryPropertyFlag checkProperties = memoryProperties;

	/* Loop throug to find all possible memory types. */
	for (uint32 i = 0; i < memory.MemoryTypeCount; ++i, score = 0)
	{
		const MemoryPropertyFlag flags = memory.MemoryTypes[i].PropertyFlags;

		/* Check if type is supported and if the required properties are supported. */
		if ((memoryTypeBits & (1 << i)) && _CrtEnumCheckFlag(flags, checkProperties))
		{
			/* See Scoring document for scoring information. */
			if (_CrtEnumCheckFlag(flags, MemoryPropertyFlag::DeviceLocal)) score += 3;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostCoherent)) score += 2;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostVisible)) ++score;
			if (preferCaching && _CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostCached)) ++score;

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