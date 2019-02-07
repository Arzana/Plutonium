#include "Graphics/Vulkan/Queue.h"
#include "Graphics/Vulkan/LogicalDevice.h"
#include "Graphics/Vulkan/Semaphore.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Core/Diagnostics/Logging.h"

using namespace Pu;

Pu::Queue::Queue(Queue && value)
	: parent(value.parent), hndl(value.hndl), index(value.index)
{
	value.hndl = nullptr;
}

Queue & Pu::Queue::operator=(Queue && other)
{
	if (this != &other)
	{
		hndl = other.hndl;
		index = other.index;
		parent = std::move(other.parent);

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::Queue::Submit(CommandBuffer & commandBuffer)
{
	const PipelineStageFlag mask = PipelineStageFlag::Transfer;
	SubmitInfo info;
	info.WaitDstStageMask = &mask;
	info.CommandBufferCount = 1;
	info.CommandBuffers = &commandBuffer.hndl;

	/* Submit command buffer. */
	VK_VALIDATE(parent.vkQueueSubmit(hndl, 1, &info, commandBuffer.submitFence->hndl), PFN_vkQueueSubmit);
	commandBuffer.state = CommandBuffer::State::Pending;
}

void Pu::Queue::Submit(const Semaphore & waitSemaphore, CommandBuffer & commandBuffer, const Semaphore & signalSemaphore)
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
	VK_VALIDATE(parent.vkQueueSubmit(hndl, 1, &info, commandBuffer.submitFence->hndl), PFN_vkQueueSubmit);
	commandBuffer.state = CommandBuffer::State::Pending;
}

void Pu::Queue::Present(const Semaphore & waitSemaphore, const Swapchain & swapchain, uint32 image)
{
	/* Create present information. */
	PresentInfo info(1, &swapchain.hndl, &image);
	info.WaitSemaphoreCount = 1;
	info.WaitSemaphores = &waitSemaphore.hndl;

	/* Present image. */
	VK_VALIDATE(parent.vkQueuePresentKHR(hndl, &info), PFN_vkQueuePresentKHR);
}

Pu::Queue::Queue(LogicalDevice &device, QueueHndl hndl, uint32 familyIndex)
	: parent(device), hndl(hndl), index(familyIndex)
{}