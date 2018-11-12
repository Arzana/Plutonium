#include "Graphics/Vulkan/Queue.h"
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Semaphore.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Core/Diagnostics/Logging.h"

using namespace Pu;

Pu::Queue::Queue(Queue && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Queue & Pu::Queue::operator=(Queue && other)
{
	if (this != &other)
	{
		hndl = other.hndl;
		parent = std::move(other.parent);

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::Queue::Submit(const Semaphore & waitSemaphore, const CommandBuffer & commandBuffer, const Semaphore & signalSemaphore)
{
	/* Create submit information. */
	const PipelineStageFlag mask = PipelineStageFlag::Transfer;
	SubmitInfo info;
	info.WaitSemaphoreCount = 1;
	info.WaitSemaphores = &waitSemaphore.hndl;
	info.WaitDstStageMask = &mask;
	info.CommandBufferCount = 1;
	info.CommandBuffers = &commandBuffer.hndl;
	info.SignalSemaphoreCount = 1;
	info.SignalSemaphores = &signalSemaphore.hndl;

	/* Submit command buffer. */
	const VkApiResult result = parent.vkQueueSubmit(hndl, 1, &info, nullptr);
	if (result != VkApiResult::Success) Log::Fatal("Unable to submit to queue!");
}

void Pu::Queue::Present(const Semaphore & waitSemaphore, const Swapchain & swapchain, uint32 image)
{
	/* Create present information. */
	PresentInfo info(1, &swapchain.hndl, &image);
	info.WaitSemaphoreCount = 1;
	info.WaitSemaphores = &waitSemaphore.hndl;

	/* Present image. */
	const VkApiResult result = parent.vkQueuePresentKHR(hndl, &info);
	if (result != VkApiResult::Success) Log::Fatal("Unable to present image from specified swapchain!");
}

Pu::Queue::Queue(LogicalDevice &device, QueueHndl hndl)
	: parent(device), hndl(hndl)
{}