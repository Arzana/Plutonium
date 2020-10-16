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
		parent = other.parent;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::Queue::WaitIdle(void) const
{
	VK_VALIDATE(parent->vkQueueWaitIdle(hndl), PFN_vkQueueWaitIdle);
}

void Pu::Queue::Submit(CommandBuffer & commandBuffer)
{
	SubmitInternal(commandBuffer, nullptr, nullptr);
}

void Pu::Queue::Submit(const Semaphore & waitSemaphore, CommandBuffer & commandBuffer, const Semaphore & signalSemaphore)
{
	SubmitInternal(commandBuffer, waitSemaphore.hndl, signalSemaphore.hndl);
}

bool Pu::Queue::Present(const Semaphore & waitSemaphore, const Swapchain & swapchain, uint32 image)
{
	/* Create present information. */
	PresentInfo info(1, &swapchain.hndl, &image);
	info.WaitSemaphoreCount = 1;
	info.WaitSemaphores = &waitSemaphore.hndl;

	/* Present image. */
	lock.lock();
	const VkApiResult result = parent->vkQueuePresentKHR(hndl, &info);
	lock.unlock();

	VK_VALIDATE(result, PFN_vkQueuePresentKHR);
	return result == VkApiResult::Success;
}

void Pu::Queue::BeginLabel(const string & name, Color color)
{
#ifdef _DEBUG
	const Vector4 clr = color.ToVector4();

	DebugUtilsLabel label;
	label.LabelName = name.c_str();
	memcpy(label.Color, &clr, sizeof(Vector4));

	lock.lock();
	parent->BeginQueueLabel(hndl, label);
	lock.unlock();
#else
	(void)name;
	(void)color;
#endif
}

void Pu::Queue::EndLabel(void)
{
#ifdef _DEBUG
	lock.lock();
	parent->EndQueueLabel(hndl);
	lock.unlock();
#endif
}

Pu::Queue::Queue(LogicalDevice &device, QueueHndl hndl, uint32 familyIndex)
	: parent(&device), hndl(hndl), index(familyIndex)
{}

void Pu::Queue::SubmitInternal(CommandBuffer & commandBuffer, SemaphoreHndl wait, SemaphoreHndl signal)
{
	/* Create submit information. */
	const PipelineStageFlags mask = PipelineStageFlags::Transfer;
	SubmitInfo info;
	info.WaitSemaphoreCount = wait != nullptr;
	info.WaitSemaphores = &wait;
	info.WaitDstStageMask = &mask;
	info.CommandBufferCount = 1;
	info.CommandBuffers = &commandBuffer.hndl;
	info.SignalSemaphoreCount = signal != nullptr;
	info.SignalSemaphores = &signal;

	/* Submit command buffer. */
	lock.lock();
	VK_VALIDATE(parent->vkQueueSubmit(hndl, 1, &info, commandBuffer.submitFence->hndl), PFN_vkQueueSubmit);
	lock.unlock();

	commandBuffer.state = CommandBuffer::State::Pending;
	commandBuffer.lastSubmitQueueFamilyID = index;
}