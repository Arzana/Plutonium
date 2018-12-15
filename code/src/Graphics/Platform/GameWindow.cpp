#include "Graphics/Platform/GameWindow.h"
#include "Core/EnumUtils.h"

using namespace Pu;

Pu::GameWindow::GameWindow(NativeWindow & native, LogicalDevice & device)
	: native(native), device(device), swapchain(nullptr), queueIndex(0)
{
	/* Make sure we update the swapchains size upon a window size change. */
	native.OnSizeChanged.Add(*this, &GameWindow::UpdateSwapchainSize);
	CreateSwapchain(native.GetClientBounds().GetSize());

	/* Get physical device and it's queue families. */
	const PhysicalDevice &physicalDevice = device.GetPhysicalDevice();
	const vector<QueueFamilyProperties> queueFamilies = physicalDevice.GetQueueFamilies();

	/* Get queue family used to present images on the swapchain. */
	for (const QueueFamilyProperties &prop : queueFamilies)
	{
		/* Check if the queue family supports graphics operation and if it supports presenting. */
		if (_CrtEnumCheckFlag(prop.Flags, QueueFlag::Graphics) &&
			native.GetSurface().QueueFamilySupportsPresenting(queueIndex, physicalDevice))
		{
			break;
		}

		++queueIndex;
	}

	if (queueIndex >= queueFamilies.size()) Log::Fatal("Cannot find queue that is viable for the swapchain!");

	/* Create new command pool. */
	pool = new CommandPool(device, queueIndex);

	/* Allocate a command buffer for each image in the swapchain. */
	for (uint32 i = 0; i < 2; i++)
	{
		buffers.push_back(pool->AllocateCommandBuffer());
	}

	/*
	Image available semaphore.
	Render finish semaphore.
	*/
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

void Pu::GameWindow::UpdateSwapchainSize(const NativeWindow &, ValueChangedEventArgs<Vector2> args)
{
	CreateSwapchain(Extent2D(ipart(args.NewValue.X), ipart(args.NewValue.Y)));
}

void Pu::GameWindow::CreateSwapchain(Extent2D size)
{
	const Swapchain *old = swapchain;

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

void Pu::GameWindow::MakeSwapchainImageWritable(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	/* Transfer present image to a writable image. */
	ImageMemoryBarrier barrier(swapchain->GetImage(curImgIdx), queueIndex);
	barrier.SrcAccessMask = AccessFlag::MemoryRead;
	barrier.DstAccessMask = AccessFlag::TransferWrite;
	barrier.NewLayout = ImageLayout::TransferDstOptimal;
	barrier.SubresourceRange = range;
	device.vkCmdPipelineBarrier(GetCommandBuffer().hndl, PipelineStageFlag::Transfer, PipelineStageFlag::Transfer,
		DependencyFlag::None, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Pu::GameWindow::MakeImagePresentable(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	/* Transfer writable image back to present mode. */
	ImageMemoryBarrier barrier(swapchain->GetImage(curImgIdx), queueIndex);
	barrier.SrcAccessMask = AccessFlag::TransferWrite;
	barrier.DstAccessMask = AccessFlag::MemoryRead;
	barrier.OldLayout = ImageLayout::TransferDstOptimal;
	barrier.NewLayout = ImageLayout::PresentSrcKhr;
	barrier.SubresourceRange = range;
	device.vkCmdPipelineBarrier(GetCommandBuffer().hndl, PipelineStageFlag::Transfer, PipelineStageFlag::BottomOfPipe,
		DependencyFlag::None, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Pu::GameWindow::BeginRender(void)
{
	/* Request new image from the swapchain. */
	curImgIdx = swapchain->NextImage(semaphores[0]);

	/* Enable command buffer. */
	GetCommandBuffer().Begin();

	/* Make swapchain image writable for following user command. */
	MakeSwapchainImageWritable();
}

void Pu::GameWindow::EndRender(void)
{
	/* Make swapchain image presentable again for the window. */
	MakeImagePresentable();

	/* End the command buffer gather. */
	GetCommandBuffer().End();

	/* Submit the command buffer to the render queue and present the queue. */
	Queue &queue = device.GetQueue(queueIndex, 0);
	queue.Submit(semaphores[0], GetCommandBuffer(), semaphores[1]);
	queue.Present(semaphores[1], *swapchain, curImgIdx);
}