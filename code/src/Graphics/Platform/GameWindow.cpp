#include "Graphics/Platform/GameWindow.h"
#include "Core/EnumUtils.h"

using namespace Pu;

Pu::GameWindow::GameWindow(NativeWindow & native, LogicalDevice & device)
	: native(native), device(device), swapchain(nullptr)
{
	/* Make sure we update the swapchains size upon a window size change. */
	native.OnSizeChanged.Add(*this, &GameWindow::UpdateSwapchainSize);
	CreateSwapchain(Extent2D(ipart(native.GetClientBounds().Size.X), ipart(native.GetClientBounds().Size.Y)));

	/* Get physical device and it's queue families. */
	const PhysicalDevice &physicalDevice = device.GetPhysicalDevice();
	const vector<QueueFamilyProperties> queueFamilies = physicalDevice.GetQueueFamilies();

	/* Get queue family used to present images on the swapchain. */
	uint32 familyIndex = 0;
	for (const QueueFamilyProperties &prop : queueFamilies)
	{
		/* Check if the queue family supports graphics operation and if it supports presenting. */
		if (_CrtEnumCheckFlag(prop.Flags, QueueFlag::Graphics) &&
			native.GetSurface().QueueFamilySupportsPresenting(familyIndex, physicalDevice))
		{
			break;
		}

		++familyIndex;
	}

	if (familyIndex >= queueFamilies.size()) Log::Fatal("Cannot find queue that is viable for the swapchain!");

	/* Create new command pool. */
	pool = new CommandPool(device, familyIndex);

	/* Allocate a command buffer for each image in the swapchain. */
	for (uint32 i = 0; i < 2; i++)
	{
		buffers.push_back(pool->AllocateCommandBuffer());
	}

	/*
	Image available semaphore.
	Wait semaphore.
	Render finish semaphore.
	*/
	semaphores.push_back(Semaphore(device));
	semaphores.push_back(Semaphore(device));
	semaphores.push_back(Semaphore(device));
}

Pu::GameWindow::~GameWindow(void)
{
	native.OnSizeChanged.Remove(*this, &GameWindow::UpdateSwapchainSize);

	semaphores.clear();
	buffers.clear();
	delete_s(pool);
	delete_s(swapchain);
}

void Pu::GameWindow::SwapBuffers(void)
{
	Queue &queue = device.GetQueue(queueIndex, 0);
	const uint32 imageIdx = swapchain->NextImage(semaphores[0]);
	queue.Submit(semaphores[1], buffers[imageIdx], semaphores[2]);
	queue.Present(semaphores[2], *swapchain, imageIdx);
}

void Pu::GameWindow::UpdateSwapchainSize(const NativeWindow &, ValueChangedEventArgs<Vector2> args)
{
	CreateSwapchain(Extent2D(ipart(args.NewValue.X), ipart(args.NewValue.Y)));
}

void Pu::GameWindow::CreateSwapchain(Extent2D size)
{
	Swapchain *old = swapchain;

	/* Create swapchain information for a general LDR color RT. */
	SwapchainCreateInfo info(native.GetSurface().hndl, size);
	info.PresentMode = PresentMode::MailBox;
	info.ImageFormat = Format::B8G8R8A8_UNORM;
	info.ImageUsage = ImageUsageFlag::ColorAttachment;
	info.Transform = SurfaceTransformFlag::Identity;
	if (old) info.OldSwapChain = swapchain->hndl;

	/* Create new swapchain. */
	swapchain = new Swapchain(device, native.GetSurface(), info);
	if (old) delete_s(old);
}

void Pu::GameWindow::BeginRender(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	uint32 i = 0;
	for (const CommandBuffer &buffer : buffers)
	{
		/* Begin command buffer. */
		const CommandBufferBeginInfo begin;
		device.vkBeginCommandBuffer(buffer.hndl, &begin);

		/* Transfer image so we can write to it. */
		ImageMemoryBarrier barrier(GetImage(i++));
		barrier.SrcAccessMask = AccessFlag::MemoryRead;
		barrier.DstAccessMask = AccessFlag::TransferWrite;
		barrier.NewLayout = ImageLayout::TransferDstOptimal;
		barrier.SrcQueueFamilyIndex = queueIndex;
		barrier.DstQueueFamilyIndex = queueIndex;
		barrier.SubresourceRange = range;
		device.vkCmdPipelineBarrier(buffer.hndl, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer,
			DependencyFlag::None, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}

void Pu::GameWindow::EndRender(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	for (uint32 i = 0; i < buffers.size(); i++)
	{
		/* Transfer image back so they can be presented. */
		ImageMemoryBarrier barrier(GetImage(i));
		barrier.SrcAccessMask = AccessFlag::TransferWrite;
		barrier.DstAccessMask = AccessFlag::MemoryRead;
		barrier.OldLayout = ImageLayout::TransferDstOptimal;
		barrier.NewLayout = ImageLayout::PresentSrcKhr;
		barrier.SrcQueueFamilyIndex = queueIndex;
		barrier.DstQueueFamilyIndex = queueIndex;
		barrier.SubresourceRange = range;
		device.vkCmdPipelineBarrier(buffers[i].hndl, PipelineStageFlag::Transfer, PipelineStageFlag::BottomOfPipe,
			DependencyFlag::None, 0, nullptr, 0, nullptr, 1, &barrier);
	}
}