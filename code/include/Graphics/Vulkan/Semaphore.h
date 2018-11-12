#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a semaphore. */
	class  Semaphore
	{
	public:
		/* Initializes a new instance of a semaphore for a specified logical device. */
		Semaphore(_In_ LogicalDevice &device);
		Semaphore(_In_ const Semaphore&) = delete;
		/* Move constructor. */
		Semaphore(_In_ Semaphore &&value);
		/* Destroys the semaphore. */
		~Semaphore(void)
		{
			Destroy();
		}

		_Check_return_ Semaphore& operator =(_In_ const Semaphore&) = delete;
		/* Move assignment. */
		_Check_return_ Semaphore& operator =(_In_ Semaphore &&other);

	private:
		friend class Swapchain;
		friend class Queue;

		LogicalDevice &parent;
		SemaphoreHndl hndl;

		void Destroy(void);
	};
}