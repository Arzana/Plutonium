#include "Graphics/Vulkan/Swapchain.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/EnumUtils.h"

Pu::Swapchain::Swapchain(LogicalDevice & device, const Surface & surface, const SwapchainCreateInfo & createInfo)
	: parent(&device), format(createInfo.ImageFormat, createInfo.ImageColorSpace), mode(createInfo.PresentMode)
{
	/* Check if the information specified is correct. */
#ifdef _DEBUG
	if (!CanCreateInternal(device.GetPhysicalDevice(), surface, createInfo, true)) Log::Fatal("Cannot create swapchain with the given arguments for the specified device or surface!");
#else 
	(void)surface;
#endif

	/* Create swapchain. */
	VK_VALIDATE(parent->vkCreateSwapchainKHR(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateSwapchainKHR);

	AquireImages(createInfo);

	/* Set the description used later to link images to render passes. */
	attachmentDesc = AttachmentDescription(createInfo.ImageFormat, ImageLayout::PresentSrcKhr, ImageLayout::PresentSrcKhr);
	attachmentDesc.LoadOp = AttachmentLoadOp::DontCare;
}

Pu::Swapchain::Swapchain(Swapchain && value)
	: parent(value.parent), hndl(value.hndl), format(value.format), attachmentDesc(value.attachmentDesc),
	images(std::move(value.images)), views(std::move(value.views)), mode(value.mode)
{
	value.hndl = nullptr;
}

Pu::Swapchain & Pu::Swapchain::operator=(Swapchain && other)
{
	if (this != &other)
	{
		Destroy();
		parent = std::move(other.parent);
		hndl = other.hndl;
		format = other.format;
		attachmentDesc = other.attachmentDesc;
		images = std::move(other.images);
		views = std::move(other.views);
		mode = other.mode;

		other.hndl = nullptr;
	}

	return *this;
}

bool Pu::Swapchain::CanCreate(const PhysicalDevice & physicalDevice, const Surface & surface, const SwapchainCreateInfo & createInfo)
{
	return CanCreateInternal(physicalDevice, surface, createInfo, false);
}

Pu::uint32 Pu::Swapchain::NextImage(const Semaphore & semaphore, uint64 timeout) const
{
	uint32 image;
	VK_VALIDATE(parent->vkAcquireNextImageKHR(parent->hndl, hndl, timeout, semaphore.hndl, nullptr, &image), PFN_vkAcquireNextImageKHR);
	return image;
}

bool Pu::Swapchain::CanCreateInternal(const PhysicalDevice & physicalDevice, const Surface & surface, const SwapchainCreateInfo & createInfo, bool raise)
{
	/* Get required checking properties. */
	const SurfaceCapabilities capabilities = surface.GetCapabilities(physicalDevice);
	const vector<SurfaceFormat> formats = surface.GetSupportedFormats(physicalDevice);
	const vector<PresentMode> presentModes = surface.GetSupportedPresentModes(physicalDevice);

	/* Perform image count and array layer checks. */
	if (capabilities.MinImageCount > createInfo.MinImageCount) return false;
	if (createInfo.ImageArrayLayers > capabilities.MaxImageArrayLayers) return false;

	/* Check if format is supported. */
	bool supported = false;
	for (const SurfaceFormat &format : formats)
	{
		if (format.Format == createInfo.ImageFormat &&
			format.ColorSpace == createInfo.ImageColorSpace)
		{
			supported = true;
			break;
		}
	}

	if (!supported)
	{
		if (raise) Log::Warning("Surface format is not supported by window surface!");
		return false;
	}

	/* Check if present mode is supported. */
	supported = false;
	for (PresentMode mode : presentModes)
	{
		if (mode == createInfo.PresentMode)
		{
			supported = true;
			break;
		}
	}

	if (!supported)
	{
		if (raise) Log::Warning("Present mode is not supported by physical device!");
		return false;
	}

	/* Check if image size is supported. */
	if (!capabilities.IsExtentAuto())
	{
		if (createInfo.ImageExtent < capabilities.MinImageExtent)
		{
			if (raise) Log::Warning("Surface image size is too small for physical device!");
			return false;
		}
		if (capabilities.MaxImageExtent < createInfo.ImageExtent)
		{
			if (raise) Log::Warning("Surface image size is too big for physical device!");
			return false;
		}
	}

	/* Check if image usage and transform are supported. */
	if (!_CrtEnumCheckFlag(capabilities.SupportedUsages, createInfo.ImageUsage))
	{
		if (raise) Log::Warning("Surface usage is not supported by the window surface!");
		return false;
	}
	if (!_CrtEnumCheckFlag(capabilities.SupportedTransforms, createInfo.Transform))
	{
		if (raise) Log::Warning("Surface transform is not supported by the window surface!");
		return false;
	}

	/* All checks passed so return true. */
	return true;
}

void Pu::Swapchain::AquireImages(const SwapchainCreateInfo & createInfo)
{
	/* Get how mnay images the swapchain has created. */
	uint32 imageCount;
	VK_VALIDATE(parent->vkGetSwapchainImagesKHR(parent->hndl, hndl, &imageCount, nullptr), PFN_vkGetSwapchainImagesKHR);

	/* Aquire the handles to all swapchain images. */
	vector<ImageHndl> handles(imageCount, nullptr);
	VK_VALIDATE(parent->vkGetSwapchainImagesKHR(parent->hndl, hndl, &imageCount, handles.data()), PFN_vkGetSwapchainImagesKHR);

	/* Create image handler and image views. */
	const Extent3D extent(createInfo.ImageExtent, 0);
	for (size_t i = 0; i < imageCount; i++)
	{
		images.emplace_back(Image(*parent, handles[i], ImageType::Image2D, createInfo.ImageFormat, extent, 0, createInfo.ImageUsage, attachmentDesc.InitialLayout, AccessFlags::MemoryRead));
		views.emplace_back(images[i], ImageViewType::Image2D, ImageAspectFlags::Color);
	}
}

void Pu::Swapchain::Destroy(void)
{
	if (hndl) parent->vkDestroySwapchainKHR(parent->hndl, hndl, nullptr);
}