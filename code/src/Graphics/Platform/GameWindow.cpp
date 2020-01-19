#include "Graphics/Platform/GameWindow.h"
#include "Core/EnumUtils.h"
#include "Graphics/Vulkan/Instance.h"
#include <imgui/include/imgui.h>
#include <imgui/include/imgui_impl_vulkan.h>

#ifdef _WIN32
#include <imgui/include/imgui_impl_win32.h>
#endif

using namespace Pu;

static void ImGuiVkValidate(VkApiResult result)
{
	Pu::ValidateVkApiResult(result, "<undefined>");
}

Pu::GameWindow::GameWindow(NativeWindow & native, LogicalDevice & device)
	: native(native), device(device), swapchain(nullptr),
	SwapchainRecreated("GameWindowSwapchainRecreated"), swapchainOutOfDate(false)
{
	/* Make sure we update the swapchains size upon a window size change. */
	native.OnSizeChanged.Add(*this, &GameWindow::OnNativeSizeChangedHandler);

	/* Non-linear sRGB is always supported so search for that one and use it. */
	for (const SurfaceFormat &format : GetSupportedFormats())
	{
		if (format.ColorSpace == ColorSpace::SRGB)
		{
			CreateSwapchain(native.GetClientBounds().GetSize(), format, true);
			break;
		}
	}

	/* Create new command pool, we're reusing command buffers so we need to enable that via a pool flag. */
	pool = new CommandPool(device, device.graphicsQueueFamily, CommandPoolCreateFlag::ResetCommandBuffer);

	/* Allocate a command buffer for each image in the swapchain. */
	for (uint32 i = 0; i < swapchain->GetImageCount(); i++)
	{
		buffers.emplace_back(pool->Allocate());
	}

	if constexpr (ImGuiAvailable)
	{
		/* Record the ImGui texture stage commands. */
		GetCommandBuffer().Begin();
		ImGui_ImplVulkan_CreateFontsTexture(GetCommandBuffer().hndl);
		GetCommandBuffer().End();

		/* Submit the command buffer. */
		device.GetGraphicsQueue(0).Submit(GetCommandBuffer());
		device.GetGraphicsQueue(0).WaitIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	/*
	Image available semaphore.
	Render finish semaphore.
	*/
	semaphores.emplace_back(Semaphore(device));
	semaphores.emplace_back(Semaphore(device));
}

Pu::GameWindow::~GameWindow(void)
{
	/* Shutdown ImGui if needed. */
	if constexpr (ImGuiAvailable) DestroyImGui();

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
	decltype(frameBuffers)::iterator it = frameBuffers.find(renderPass.hndl);
	if (it != frameBuffers.end())
	{
		Log::Warning("Recreating framebuffers for renderpass!");
		for (const Framebuffer *cur : it->second) delete cur;
		frameBuffers.erase(it);
	}

	/* Get dimensions of the window and create buffer for storage. */
	const Extent2D dimensions = native.GetClientBounds().GetSize();
	vector<Framebuffer*> tmpFramebuffers;

	/* Create a framebuffer for each swapchain image. */
	for (const ImageView &cur : swapchain->views)
	{
		vector<const ImageView*> attachments;
		attachments.emplace_back(&cur);

		/* The additional views need to be added afterwards to comply with the attachment index order. */
		for (const ImageView *view : views) attachments.emplace_back(view);
		tmpFramebuffers.emplace_back(new Framebuffer(renderPass, dimensions, attachments));
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

const vector<SurfaceFormat>& Pu::GameWindow::GetSupportedFormats(void) const
{
	/* This operation is quite slow so we use a cache to speed it up, this list only needs to be updated once anyway. */
	if (supportedFormats.empty()) supportedFormats = native.GetSurface().GetSupportedFormats(device.GetPhysicalDevice());
	return supportedFormats;
}

void Pu::GameWindow::SetColorSpace(ColorSpace colorSpace)
{
	/* Make sure that we don't change if it's not needed. */
	if (swapchain)
	{
		if (colorSpace == swapchain->GetColorSpace()) return;
	}

	/* Check if the desired format is supported by the monitor. */
	for (const SurfaceFormat &format : GetSupportedFormats())
	{
		if (format.ColorSpace == colorSpace)
		{
			/* Make sure non of the resources are in use. */
			Log::Warning("Changing window color space, this might cause lag!");
			ReCreateSwapchain(native.GetClientBounds().GetSize(), format, SwapchainReCreatedEventArgs(false, true, false));
			return;
		}
	}

	Log::Warning("Color space (%s) is not supported by the surface!", to_string(colorSpace));
}

void Pu::GameWindow::SetColorSpace(const SurfaceFormat & format)
{
	/* Make sure that we don't change if it's not needed. */
	if (swapchain)
	{
		if (format == swapchain->GetFormat()) return;
	}

	/* Check if the desired format is supported by the monitor. */
	for (const SurfaceFormat &checkFormat : GetSupportedFormats())
	{
		if (format == checkFormat)
		{
			Log::Warning("Changing window format and color space, this might cause loag!");
			ReCreateSwapchain(native.GetClientBounds().GetSize(), format, SwapchainReCreatedEventArgs(false, true, false));
			return;
		}
	}

	Log::Warning("Format %s is not supported by the surface!", ((string)format).c_str());
}

void Pu::GameWindow::OnNativeSizeChangedHandler(const NativeWindow &, ValueChangedEventArgs<Vector2> args)
{
	/* Make sure this is not called before the swapchain is even created, rare but who knows. */
	if (swapchain)
	{
		Log::Warning("Window is being resized, this might cause lag!");
		ReCreateSwapchain(Extent2D(ipart(args.NewValue.X), ipart(args.NewValue.Y)), swapchain->format, SwapchainReCreatedEventArgs(true, false, false));
	}
}

void Pu::GameWindow::ReCreateSwapchain(Extent2D size, SurfaceFormat format, const SwapchainReCreatedEventArgs & args)
{
	/* The swapchain images or the framebuffers might still be in use by the command buffers, so wait until they're available again. */
	Finalize();

	/* Free all framebuffers bound to the native window size. */
	DestroyFramebuffers();

	/* Update the swapchain images. */
	CreateSwapchain(size, format, false);

	/* Notify the subscribers of the change in size or format. */
	SwapchainRecreated.Post(*this, args);
}

void Pu::GameWindow::CreateSwapchain(Extent2D size, SurfaceFormat format, bool firstCall)
{
	/* Update the orthographics matrix used to convert the coordinates from viewport space to clip space. */
	ortho = Matrix::CreateOrtho(static_cast<float>(size.Width), static_cast<float>(size.Height), 0.0f, 1.0f);

	/* Create swapchain information for a general LDR color RT, just pick the first available format. */
	SwapchainCreateInfo info(native.GetSurface().hndl, size);
	info.PresentMode = PresentMode::MailBox;
	info.ImageColorSpace = format.ColorSpace;
	info.ImageFormat = format.Format;
	info.ImageUsage = ImageUsageFlag::ColorAttachment | ImageUsageFlag::TransferDst;
	info.Transform = SurfaceTransformFlag::Identity;
	if (swapchain) info.OldSwapChain = swapchain->hndl;

	/* Create new swapchain or replace the old one. */
	if (swapchain) *swapchain = Swapchain(device, native.GetSurface(), info);
	else swapchain = new Swapchain(device, native.GetSurface(), info);
	Log::Message("Created %s swapchain for %ux%u.", to_string(format.ColorSpace), size.Width, size.Height);

	/* Update the window size and minimum images for ImGui if needed. */
	if constexpr (ImGuiAvailable)
	{
		/* Destroy the old Vulkan handles if this is the second time we're creating the swapchain. */
		if (!firstCall) DestroyImGui();
		ImGui::GetIO().DisplaySize = size;

		/* The only attachment is the swapchain image, we're calling this after our own draw calls so we want to load. */
		AttachmentDescription attachment(info.ImageFormat, ImageLayout::PresentSrcKhr, ImageLayout::PresentSrcKhr);
		attachment.LoadOp = AttachmentLoadOp::Load;

		const AttachmentReference reference(0, ImageLayout::ColorAttachmentOptimal);
		const SubpassDescription subpass(reference);

		SubpassDependency dependency(SubpassExternal, 0);
		dependency.SrcStageMask = PipelineStageFlag::ColorAttachmentOutput;
		dependency.DstStageMask = PipelineStageFlag::ColorAttachmentOutput;
		dependency.DstAccessMask = AccessFlag::ColorAttachmentWrite;

		const RenderPassCreateInfo renderPassInfo(attachment, subpass, dependency);
		VK_VALIDATE(device.vkCreateRenderPass(device.hndl, &renderPassInfo, nullptr, &imGuiRenderPass), PFN_vkCreateRenderPass);

		/*
		These values predict the amount of objects created for ImGui,
		If they need to be changed then all of them need to allign.
		*/
		const vector<DescriptorPoolSize> poolSizes =
		{
			{ DescriptorType::UniformBuffer, 2 },
			{ DescriptorType::CombinedImageSampler, 1 }
		};

		const DescriptorPoolCreateInfo descriptorPoolInfo(3, poolSizes);
		VK_VALIDATE(device.vkCreateDescriptorPool(device.hndl, &descriptorPoolInfo, nullptr, &imGuiDescriptorPool), PFN_vkCreateDescriptorPool);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = device.parent->parent->hndl;
		initInfo.PhysicalDevice = device.parent->hndl;
		initInfo.Device = device.hndl;
		initInfo.QueueFamily = device.graphicsQueueFamily;
		initInfo.Queue = device.GetGraphicsQueue(0).hndl;
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = imGuiDescriptorPool;
		initInfo.Allocator = nullptr;
		initInfo.MinImageCount = info.MinImageCount;
		initInfo.ImageCount = static_cast<uint32>(swapchain->GetImageCount());
		initInfo.CheckVkResultFn = ImGuiVkValidate;
		ImGui_ImplVulkan_Init(&initInfo, imGuiRenderPass);

		/* We need command buffers to do this and we don't have one yet on the first call. */
		if (!firstCall)
		{
			/* Record the ImGui texture stage commands. */
			GetCommandBuffer().Begin();
			ImGui_ImplVulkan_CreateFontsTexture(GetCommandBuffer().hndl);
			GetCommandBuffer().End();

			/* Submit the command buffer. */
			device.GetGraphicsQueue(0).Submit(GetCommandBuffer());
		}

		/* Create the framebuffers needed for ImGui, this will probably be two. */
		vector<Framebuffer*> imGuiFrameBuffers;
		const Extent2D dimensions = native.GetClientBounds().GetSize();
		for (const ImageView &cur : swapchain->views)
		{
			imGuiFrameBuffers.emplace_back(new Framebuffer(device, imGuiRenderPass, dimensions, cur.hndl));
		}

		frameBuffers.emplace(imGuiRenderPass, imGuiFrameBuffers);

		if (!firstCall)
		{
			/*
			Wait for the tasks to complete and then delete the pending objects.
			We're doing this as late as possible (excluding actually setting up a synchronizer).
			*/
			device.GetGraphicsQueue(0).WaitIdle();
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}
}

void Pu::GameWindow::MakeSwapchainImageWritable(void)
{
	static const ImageSubresourceRange range;

	/* Transfer present image to a writable image. */
	GetCommandBuffer().MemoryBarrier(GetCurrentImage(), PipelineStageFlag::Transfer, PipelineStageFlag::Transfer, ImageLayout::PresentSrcKhr, AccessFlag::TransferWrite, range);
}

void Pu::GameWindow::MakeImagePresentable(void)
{
	static const ImageSubresourceRange range;

	/* Transfer writable image back to present mode. */
	GetCommandBuffer().MemoryBarrier(GetCurrentImage(), PipelineStageFlag::Transfer, PipelineStageFlag::BottomOfPipe, ImageLayout::PresentSrcKhr, AccessFlag::MemoryRead, range);
}

void Pu::GameWindow::BeginRender(void)
{
	/* Make sure that we don't work with an old swapchain. */
	if (swapchainOutOfDate)
	{
		device.WaitIdle();
		swapchainOutOfDate = false;
		ReCreateSwapchain(native.GetClientBounds().GetSize(), swapchain->format, SwapchainReCreatedEventArgs(false, false, true));
	}

	/* Request new image from the swapchain. */
	curImgIdx = swapchain->NextImage(semaphores[0]);

	/* Enable command buffer. */
	GetCommandBuffer().Begin();

	/* Initialize the ImGui frame if needed. */
	if constexpr (ImGuiAvailable)
	{
		ImGui_ImplVulkan_NewFrame();
#ifdef _WIN32
		ImGui_ImplWin32_NewFrame();
#endif

		ImGui::NewFrame();
	}

	/* Make swapchain image writable for following user command. */
	MakeSwapchainImageWritable();
}

void Pu::GameWindow::EndRender(void)
{
	CommandBuffer &cmdBuf = GetCommandBuffer();

	/* End the ImGui frame if needed. */
	if constexpr (ImGuiAvailable)
	{
		static vector<ClearValue> clearValues = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		const Framebuffer &imGuiFramebuffer = *frameBuffers[imGuiRenderPass][curImgIdx];

		ImGui::Render();
		cmdBuf.AddLabel("ImGui", Color::CodGray());
		cmdBuf.BeginRenderPassInternal(imGuiRenderPass, clearValues, imGuiFramebuffer, imGuiFramebuffer.area, SubpassContents::Inline);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf.hndl);
		cmdBuf.EndRenderPass();
		cmdBuf.EndLabel();
	}

	/* Make swapchain image presentable again for the window. */
	MakeImagePresentable();

	/* End the command buffer gather. */
	cmdBuf.End();

	/* Submit the command buffer to the render queue and present the queue. */
	Queue &queue = device.GetGraphicsQueue(0);
	queue.BeginLabel(u8"GameWindow Render", Color::Blue());
	queue.Submit(semaphores[0], cmdBuf, semaphores[1]);
	swapchainOutOfDate = !queue.Present(semaphores[1], *swapchain, curImgIdx);
	queue.EndLabel();
}

void Pu::GameWindow::DestroyImGui(void)
{
	ImGui_ImplVulkan_Shutdown();

	for (const auto &[key, framebuffers] : frameBuffers)
	{
		if (key == imGuiRenderPass)
		{
			for (const Framebuffer *cur : framebuffers) delete cur;
			frameBuffers.erase(key);
			break;
		}
	}

	device.vkDestroyRenderPass(device.hndl, imGuiRenderPass, nullptr);
	device.vkDestroyDescriptorPool(device.hndl, imGuiDescriptorPool, nullptr);
}

void Pu::GameWindow::DestroyFramebuffers(void)
{
	/* One is with a capital and one is without, not the best naming... */
	for (const auto &[key, framebuffers] : frameBuffers)
	{
		for (const Framebuffer *cur : framebuffers) delete cur;
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