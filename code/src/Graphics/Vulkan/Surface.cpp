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

#ifdef _WIN32
Pu::Surface::Surface(VulkanInstance & parent, HINSTANCE hinstance, HWND hwnd)
	: parent(parent)
{
	const Win32SurfaceCreateInfo info(hinstance, hwnd);

	const VkApiResult result = parent.vkCreateWin32SurfaceKHR(parent.hndl, &info, nullptr, &hndl);
	if (result != VkApiResult::Success) Log::Fatal("Unable to create surface for Win32 window!");
}
#endif

void Pu::Surface::Destroy(void)
{
	if (hndl) parent.vkDestroySurfaceKHR(parent.hndl, hndl, nullptr);
}