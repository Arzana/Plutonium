#include "Graphics/Vulkan/Surface.h"
#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

Pu::Surface::Surface(Surface && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Surface & Pu::Surface::operator=(Surface && other)
{
	if (this != &other)
	{
		Destroy();
		parent = std::move(other.parent);
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

SurfaceCapabilities Pu::Surface::GetCapabilities(const PhysicalDevice & physicalDevice) const
{
	/* Request capabilities. */
	SurfaceCapabilities capabilities;
	VK_VALIDATE(parent.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.hndl, hndl, &capabilities), PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	return capabilities;
}

vector<SurfaceFormat> Pu::Surface::GetSupportedFormats(const PhysicalDevice & physicalDevice) const
{
	/* Query amount of formats specified. */
	uint32 count;
	parent.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.hndl, hndl, &count, nullptr);

	/* Early out if no formats are specified. */
	if (count < 1) return vector<SurfaceFormat>();

	/* Query formats. */
	vector<SurfaceFormat> result(count);
	parent.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.hndl, hndl, &count, result.data());
	return result;
}

vector<PresentMode> Pu::Surface::GetSupportedPresentModes(const PhysicalDevice & physicalDevice) const
{
	/* Query the amount of present modes. */
	uint32 count;
	parent.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.hndl, hndl, &count, nullptr);

	/* Early out if no present modes are supported. */
	if (count < 1) return vector<PresentMode>();

	vector<PresentMode> result(count);
	parent.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.hndl, hndl, &count, result.data());
	return result;
}

bool Pu::Surface::QueueFamilySupportsPresenting(uint32 queueFamilyIndex, const PhysicalDevice & physicalDevice) const
{
	Bool32 result;
	parent.vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.hndl, queueFamilyIndex, hndl, &result);
	return result;
}

#ifdef _WIN32
Pu::Surface::Surface(VulkanInstance & parent, HINSTANCE hinstance, HWND hwnd)
	: parent(parent)
{
	/* Create new surface. */
	const Win32SurfaceCreateInfo info(hinstance, hwnd);
	VK_VALIDATE(parent.vkCreateWin32SurfaceKHR(parent.hndl, &info, nullptr, &hndl), PFN_vkCreateWin32SurfaceKHR);
}
#endif

void Pu::Surface::Destroy(void)
{
	if (hndl) parent.vkDestroySurfaceKHR(parent.hndl, hndl, nullptr);
}