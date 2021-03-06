#pragma once
#include "Core/String.h"
#include "VulkanGlobals.h"
#include "Graphics/Color.h"

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

		/* Gets the queue's family index. */
		_Check_return_ inline uint32 GetFamilyIndex(void) const 
		{
			return index;
		}

		/* Wait for the queue to complete its outstanding operations. */
		void WaitIdle(void) const;
		/* Submits the commands in the specified command buffer to the queue. */
		void Submit(_In_ CommandBuffer &commandBuffer);
		/* Submits the commands in the specified command buffer to the queue. */
		void Submit(_In_ const Semaphore &waitSemaphore, _In_ CommandBuffer &commandBuffer, _In_ const Semaphore &signalSemaphore);
		/* Presents the image to the swapchain after the semaphore has completed, returns whether this was allowed. */
		_Check_return_ bool Present(_In_ const Semaphore &waitSemaphore, _In_ const Swapchain &swapchain, _In_ uint32 image);
		/* Starts a debug label with a specific name and color (only active on debug). */
		void BeginLabel(_In_ const string &name, _In_ Color color);
		/* End a the last added debug label (only active on debug). */
		void EndLabel(void);

	private:
		friend class LogicalDevice;
		friend class GameWindow;

		QueueHndl hndl;
		uint32 index;
		LogicalDevice *parent;
		std::mutex lock;

		Queue(LogicalDevice &device, QueueHndl hndl, uint32 familyIndex);

		void SubmitInternal(CommandBuffer &commandBuffer, SemaphoreHndl wait, SemaphoreHndl signal);
	};
}