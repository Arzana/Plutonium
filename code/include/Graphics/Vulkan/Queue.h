#pragma once
#include "VulkanGlobals.h"

namespace Pu
{
	class LogicalDevice;
	class Semaphore;
	class Swapchain;
	class CommandBuffer;

	/* Defines a logical device command queue. */
	class Queue
	{
	public:
		Queue(_In_ const Queue&) = delete;
		/* Move constructor. */
		Queue(_In_ Queue &&value);

		_Check_return_ Queue& operator =(_In_ const Queue&) = delete;
		/* Move assignment. */
		_Check_return_ Queue& operator =(_In_ Queue &&other);

		/* Submits the commands in the specified command pool to the queue. */
		void Submit(_In_ const Semaphore &waitSemaphore, _In_ CommandBuffer &commandBuffer, _In_ const Semaphore &signalSemaphore);
		/* Presents the image to the swapchain ofter the semaphore has completed. */
		void Present(_In_ const Semaphore &waitSemaphore, _In_ const Swapchain &swapchain, _In_ uint32 image);

	private:
		friend class LogicalDevice;

		QueueHndl hndl;
		LogicalDevice &parent;

		Queue(LogicalDevice &device, QueueHndl hndl);
	};
}