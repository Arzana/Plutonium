#pragma once
#include "LogicalDevice.h"
#include "Surface.h"

namespace Pu
{
	/* Defines a swapchain. */
	class Swapchain
	{
	public:
		/* Initializes a new instance of a swapchain for a specific device, physical device and for a specific surface with specific arguments. */
		Swapchain(_In_ LogicalDevice &device, _In_ const PhysicalDevice &physicalDevice, _In_ const Surface &surface, _In_ SwapchainCreateInfo &createInfo);
		Swapchain(_In_ const Swapchain&) = delete;
		/* Move constructor. */
		Swapchain(_In_ Swapchain &&value);
		/* Destroys the swapchain. */
		~Swapchain(void)
		{
			Destroy();
		}

		_Check_return_ Swapchain& operator =(_In_ const Swapchain&) = delete;
		/* Move assignment. */
		_Check_return_ Swapchain& operator =(_In_ Swapchain &&other);

		/* Checks whether the specified physical device and surface support the specified swapchain. */
		_Check_return_ static bool CanCreate(_In_ const PhysicalDevice &physicalDevice, _In_ const Surface &surface, _In_ SwapchainCreateInfo &createInfo);

	private:
		LogicalDevice &parent;
		SwapchainHndl hndl;

		void Destroy(void);
	};
}