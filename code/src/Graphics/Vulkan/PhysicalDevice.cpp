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

LogicalDevice Pu::PhysicalDevice::CreateLogicalDevice(const DeviceCreateInfo * createInfo) const
{
	/* Create new logical device. */
	DeviceHndl device;
	const VkApiResult result = parent.vkCreateDevice(hndl, createInfo, nullptr, &device);

	/* Check for errors. */
	if (result != VkApiResult::Success) Log::Fatal("Unable to create logical device!");

	/* Log creation. */
	const auto[major, minor, patch] = GetVulkanVersion();
	Log::Message("Created new Vulkan v%u.%u.%u logical device on %s", major, minor, patch, GetName());

	return LogicalDevice(parent.GetHandle(), device);
}

Pu::PhysicalDevice::PhysicalDevice(VulkanInstance & parent, PhysicalDeviceHndl hndl)
	: hndl(hndl), parent(parent)
{
	/* On destroy check and query the properties for fast access later. */
	parent.OnDestroy.Add(*this, &PhysicalDevice::OnParentDestroyed);
	parent.vkGetPhysicalDeviceProperties(hndl, &properties);
	parent.vkGetPhysicalDeviceFeatures(hndl, &features);
}

void Pu::PhysicalDevice::OnParentDestroyed(const VulkanInstance &, EventArgs)
{
	Log::Warning("Vulkan instance is deleted before physical device, this caused the physical device to be invalid!");
	hndl = nullptr;
}