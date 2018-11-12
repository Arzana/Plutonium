#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "PhysicalDevice.h"

namespace Pu
{
	class VulkanInstance;

	class Surface
	{
	public:
		Surface(_In_ const Surface&) = delete;
		/* Move constructor. */
		Surface(_In_ Surface &&value);
		/* Destroys the surface. */
		~Surface(void)
		{
			Destroy();
		}

		_Check_return_ Surface& operator =(_In_ const Surface&) = delete;
		/* Move assignment. */
		_Check_return_ Surface& operator =(_In_ Surface &&other);

		/* Gets the capabilities of a surface for a specific physical device. */
		_Check_return_ SurfaceCapabilities GetCapabilities(_In_ const PhysicalDevice &physicalDevice) const;
		/* Gets the supported surface formats for a specific physical device. */
		_Check_return_ vector<SurfaceFormat> GetSupportedFormats(_In_ const PhysicalDevice &physicalDevice) const;
		/* Gets the supported present modes for a specified physical device. */
		_Check_return_ vector<PresentMode> GetSupportedPresentModes(_In_ const PhysicalDevice &physicalDevice) const;
		/* Checks whether a specific queue family of this physical device supports presenting images to the specified surface. */
		/*Checks whether a specified queue family supports presenting images to this surface with the specified physical device. */
		_Check_return_ bool QueueFamilySupportsPresenting(_In_ uint32 queueFamilyIndex, _In_ const PhysicalDevice &physicalDevice) const;

	private:
		friend class PhysicalDevice;
		friend class GameWindow;
		friend class Win32Window;

		SurfaceHndl hndl;
		VulkanInstance &parent;

#ifdef _WIN32
		Surface(VulkanInstance &parent, HINSTANCE hinstance, HWND hwnd);
#endif

		void Destroy(void);
	};
}