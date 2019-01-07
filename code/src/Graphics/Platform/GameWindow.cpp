#include "Graphics/Platform/GameWindow.h"
#include "Core/EnumUtils.h"

using namespace Pu;

Pu::GameWindow::GameWindow(NativeWindow & native, LogicalDevice & device)
	: native(native), device(device), swapchain(nullptr), queueIndex(device.graphicsQueueFamily)
{
	/* Make sure we update the swapchains size upon a window size change. */
	native.OnSizeChanged.Add(*this, &GameWindow::OnNativeSizeChangedHandler);
	CreateSwapchain(native.GetClientBounds().GetSize());

	/* Create new command pool. */
	pool = new CommandPool(device, queueIndex);

	/* Allocate a command buffer for each image in the swapchain. */
	for (uint32 i = 0; i < swapchain->GetImageCount(); i++)
	{
		buffers.push_back(pool->Allocate());
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
	native.OnSizeChanged.Remove(*this, &GameWindow::OnNativeSizeChangedHandler);
	DestroyFramebuffers();

	semaphores.clear();
	buffers.clear();
	delete pool;
	delete swapchain;
}

void Pu::GameWindow::CreateFrameBuffers(const Renderpass & renderPass, const vector<const ImageView*> & views)
{
	/* Make sure we don't create too many. */
	if (frameBuffers.find(renderPass.hndl) != frameBuffers.end())
	{
		Log::Warning("Attempting to create framebuffers for render pass that already has framebuffers defined!");
		return;
	}

	/* Get dimensions of the window and create buffer for storage. */
	const Extent2D dimensions = native.GetClientBounds().GetSize();
	vector<Framebuffer*> tmpFramebuffers;

	/* Create a framebuffer for each swapchain image. */
	for (const ImageView &cur : swapchain->views)
	{
		vector<const ImageView*> attachments(views);
		attachments.emplace_back(&cur);
		tmpFramebuffers.emplace_back(new Framebuffer(device, renderPass, dimensions, attachments));
	}

	/* Add all framebuffers of the render pass to the buffer. */
	frameBuffers.emplace(renderPass.hndl, tmpFramebuffers);
}

const Framebuffer & Pu::GameWindow::GetCurrentFramebuffer(const Renderpass & renderPass) const
{
	std::map<RenderPassHndl, vector<Framebuffer*>>::const_iterator it = frameBuffers.find(renderPass.hndl);
	if (it == frameBuffers.end()) Log::Fatal("Attempting to retrieve framebuffer for unknown render pass!");

	return *it->second[curImgIdx];
}

void Pu::GameWindow::OnNativeSizeChangedHandler(const NativeWindow &, ValueChangedEventArgs<Vector2> args)
{
	Log::Warning("Window is being resized, this might cause lag!");

	/* The swapchain images or the framebuffers might still be in use by the command buffers, so wait until they're available again. */
	Finalize();

	/* Update the swapchain images. */
	CreateSwapchain(Extent2D(ipart(args.NewValue.X), ipart(args.NewValue.Y)));

	/* Free all framebuffers bound to the native window size. */
	DestroyFramebuffers();
}

void Pu::GameWindow::CreateSwapchain(Extent2D size)
{
	const Swapchain *old = swapchain;

	/* Create swapchain information for a general LDR color RT. */
	SwapchainCreateInfo info(native.GetSurface().hndl, size);
	info.PresentMode = PresentMode::MailBox;
	info.ImageFormat = Format::B8G8R8A8_UNORM;
	info.ImageUsage = ImageUsageFlag::ColorAttachment | ImageUsageFlag::TransferDst;
	info.Transform = SurfaceTransformFlag::Identity;
	if (old) info.OldSwapChain = swapchain->hndl;

	/* Create new swapchain. */
	swapchain = new Swapchain(device, native.GetSurface(), info);
	if (old) delete old;
}

void Pu::GameWindow::MakeSwapchainImageWritable(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	/* Transfer present image to a writable image. */
	GetCommandBuffer().MemoryBarrier(GetCurrentImage(), PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, DependencyFlag::None,
		ImageLayout::PresentSrcKhr, AccessFlag::TransferWrite, queueIndex, range);
}

void Pu::GameWindow::MakeImagePresentable(void)
{
	static const ImageSubresourceRange range(ImageAspectFlag::Color);

	/* Transfer writable image back to present mode. */
	GetCommandBuffer().MemoryBarrier(GetCurrentImage(), PipelineStageFlag::Transfer, PipelineStageFlag::BottomOfPipe, DependencyFlag::None,
		ImageLayout::PresentSrcKhr, AccessFlag::MemoryRead, queueIndex, range);
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

	CommandBuffer &cmdBuf = GetCommandBuffer();

	/* End the command buffer gather. */
	cmdBuf.End();

	/* Submit the command buffer to the render queue and present the queue. */
	Queue &queue = device.GetQueue(queueIndex, 0);
	queue.Submit(semaphores[0], cmdBuf, semaphores[1]);
	queue.Present(semaphores[1], *swapchain, curImgIdx);
}

void Pu::GameWindow::DestroyFramebuffers(void)
{
	/* One is with a capital and one is without, not the best naming... */
	for (const auto &[key, framebuffers] : frameBuffers)
	{
		for (size_t i = 0; i < framebuffers.size(); i++) delete framebuffers[i];
	}

	frameBuffers.clear();
}

void Pu::GameWindow::Finalize(void)
{
	/* Wait until all pending commandbuffers are done. */
	vector<const Fence*> fences;
	for (const CommandBuffer &buffer : buffers)
	{
		if (buffer.state == CommandBuffer::State::Pending) fences.emplace_back(buffer.submitFence);
	}

	if (fences.size() > 0) Fence::WaitAll(device, fences);
}