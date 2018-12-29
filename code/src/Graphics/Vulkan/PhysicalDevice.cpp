#include "Graphics/Vulkan/PhysicalDevice.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

Pu::PhysicalDevice::PhysicalDevice(PhysicalDevice && value)
	: PhysicalDevice(value.parent, value.hndl)
{
	value.hndl = nullptr;
}

Pu::PhysicalDevice::~PhysicalDevice(void)
{
	parent.OnDestroy.Remove(*this, &PhysicalDevice::OnParentDestroyed);
}

PhysicalDevice & Pu::PhysicalDevice::operator=(PhysicalDevice && other)
{
	if (this != &other)
	{
		parent.OnDestroy.Remove(*this, &PhysicalDevice::OnParentDestroyed);

		hndl = other.hndl;
		parent = std::move(other.parent);
		parent.OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);

		other.hndl = nullptr;
	}

	return *this;
}

vector<QueueFamilyProperties> Pu::PhysicalDevice::GetQueueFamilies(void) const
{
	/* Query the amount of queue families defined. */
	uint32 count;
	parent.vkGetPhysicalDeviceQueueFamilyProperties(hndl, &count, nullptr);

	/* Early out if the count is less than one. */
	if (count < 1) return vector<QueueFamilyProperties>();

	/* Query all queue familie properties. */
	vector<QueueFamilyProperties> result(count);
	parent.vkGetPhysicalDeviceQueueFamilyProperties(hndl, &count, result.data());

	return result;
}

LogicalDevice * Pu::PhysicalDevice::CreateLogicalDevice(const DeviceCreateInfo * createInfo) const
{
	/* Create new logical device. */
	DeviceHndl device;
	VK_VALIDATE(parent.vkCreateDevice(hndl, createInfo, nullptr, &device), PFN_vkCreateDevice);

	/* Log creation. */
	const auto[major, minor, patch] = GetVulkanVersion();
	Log::Message("Created new Vulkan v%u.%u.%u logical device on %s", major, minor, patch, GetName());

	return new LogicalDevice(const_cast<PhysicalDevice&>(*this), device, createInfo->QueueCreateInfoCount, createInfo->QueueCreateInfos);
}

vector<ExtensionProperties> Pu::PhysicalDevice::GetSupportedExtensions(const char * layer) const
{
	/* Query the amount of properties defined. */
	uint32 count;
	parent.vkEnumerateDeviceExtensionProperties(hndl, layer, &count, nullptr);

	/* Realy out if the count is less than one. */
	if (count < 1) return vector<ExtensionProperties>();

	/* Query all extension properties. */
	vector<ExtensionProperties> result(count);
	parent.vkEnumerateDeviceExtensionProperties(hndl, layer, &count, result.data());

	return result;
}

bool Pu::PhysicalDevice::IsExtensionSupported(const char * extension) const
{
	const vector<ExtensionProperties> props = GetSupportedExtensions(nullptr);
	return props.contains(extension, [](const ExtensionProperties &prop, const char *userParam)
	{
		return !strcmp(prop.ExtensionName, userParam);
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

bool Pu::PhysicalDevice::SupportsPlutonium(void) const
{
	return IsExtensionSupported(u8"VK_KHR_swapchain");
}

Pu::PhysicalDevice::PhysicalDevice(VulkanInstance & parent, PhysicalDeviceHndl hndl)
	: hndl(hndl), parent(parent)
{
	/* On destroy check and query the properties for fast access later. */
	parent.OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);
	parent.vkGetPhysicalDeviceProperties(hndl, &properties);
	parent.vkGetPhysicalDeviceFeatures(hndl, &features);
	parent.vkGetPhysicalDeviceMemoryProperties(hndl, &memory);
}

bool Pu::PhysicalDevice::GetBestMemoryType(uint32 memoryTypeBits, MemoryPropertyFlag memoryProperties, uint32 & index)
{
	index = maxv<uint32>();

	/* Loop throug to find all possible memory types. */
	for (uint32 i = 0, highestScore = 0; i < memory.MemoryTypeCount; ++i)
	{
		const MemoryPropertyFlag flags = memory.MemoryTypes[i].PropertyFlags;

		/* Check if type is supported and if the required properties are supported. */
		if ((memoryTypeBits & (1 << i)) && _CrtEnumCheckFlag(flags, memoryProperties))
		{
			/*
			- DeviceLocal is fast memory so that scores a point.
			- HostCoherent means less commands when caching so that scores a point.
			- HostCached means more work for the host so gain a point if it doesn't have that.
			- HostVisible is slow as the CPU and GPU need access so gain a point if it doesn't have that.
			*/
			uint32 score = 0;
			if (_CrtEnumCheckFlag(flags, MemoryPropertyFlag::DeviceLocal)) ++score;
			if (_CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostCoherent)) ++score;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostCached)) ++score;
			if (!_CrtEnumCheckFlag(flags, MemoryPropertyFlag::HostVisible)) ++score;

			if (score > highestScore)
			{
				highestScore = score;
				index = i;
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