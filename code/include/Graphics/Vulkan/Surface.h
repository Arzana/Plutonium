#pragma once
#include "VulkanGlobals.h"
#include <sal.h>

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