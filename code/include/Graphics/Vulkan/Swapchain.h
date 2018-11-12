#pragma once
#include "LogicalDevice.h"
#include "Surface.h"
#include "Semaphore.h"

namespace Pu
{
	/* Defines a swapchain. */
	class Swapchain
	{
	public:
		/* Initializes a new instance of a swapchain for a specific device, physical device and for a specific surface with specific arguments. */
		Swapchain(_In_ LogicalDevice &device, _In_ const Surface &surface, _In_ const SwapchainCreateInfo &createInfo);
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
		_Check_return_ static bool CanCreate(_In_ const PhysicalDevice &physicalDevice, _In_ const Surface &surface, _In_ const SwapchainCreateInfo &createInfo);

		/* Acquires the next image index available in the swapchain. */
		_Check_return_ uint32 NextImage(_In_ const Semaphore &semaphore, _In_opt_ uint64 timeout = 16666666) const;

		/* Gets the image handle at the specified index. */
		_Check_return_ inline ImageHndl GetImage(_In_ uint32 index) const
		{
			return images.at(index);
		}

	private:
		friend class Queue;
		friend class GameWindow;

		LogicalDevice &parent;
		SwapchainHndl hndl;
		vector<ImageHndl> images;

		void Destroy(void);
	};
}