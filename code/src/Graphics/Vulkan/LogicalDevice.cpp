#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Loader.h"

using namespace Pu;

Pu::LogicalDevice::LogicalDevice(LogicalDevice && value)
	: parent(value.parent), hndl(value.hndl), vkDestroyDevice(value.vkDestroyDevice)
{
	value.parent = nullptr;
	value.hndl = nullptr;
	value.vkDestroyDevice = nullptr;
}

LogicalDevice & Pu::LogicalDevice::operator=(LogicalDevice && other)
{
	if (this != &other)
	{
		Destory();
		parent = other.parent;
		hndl = other.hndl;
		vkDestroyDevice = other.vkDestroyDevice;

		other.parent = nullptr;
		other.hndl = nullptr;
		other.vkDestroyDevice = nullptr;
	}

	return *this;
}

Pu::LogicalDevice::LogicalDevice(InstanceHndl parent, DeviceHndl hndl)
	: parent(parent), hndl(hndl)
{
	VK_LOAD_DEVICE_PROC(parent, hndl, vkDestroyDevice);
}

void Pu::LogicalDevice::Destory(void)
{
	if (hndl)
	{
		vkDestroyDevice(hndl, nullptr);
	}
}