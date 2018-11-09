#pragma once
#include "PhysicalDevice.h"

#ifdef _WIN32
#include <Windows.h>
#endif

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

	private:
		friend class PhysicalDevice;
		friend class Win32Window;

		SurfaceHndl hndl;
		VulkanInstance &parent;

#ifdef _WIN32
		Surface(VulkanInstance &parent, HINSTANCE hinstance, HWND hwnd);
#endif

		void Destroy(void);
	};
}