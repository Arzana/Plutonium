#include "Graphics/Vulkan/Swapchain.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/EnumUtils.h"

Pu::Swapchain::Swapchain(LogicalDevice & device, const Surface & surface, const SwapchainCreateInfo & createInfo)
	: parent(device), format(createInfo.ImageFormat)
{
	/* Check if the information specified is correct. */
	if (!CanCreate(device.GetPhysicalDevice(), surface, createInfo)) Log::Fatal("Cannot create swapchain with the given arguments for the specified device or surface!");

	/* Create swapchain. */
	VK_VALIDATE(parent.vkCreateSwapchainKHR(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateSwapchainKHR);

	/* Acquire images within swapchain. */
	uint32 imageCount;
	VK_VALIDATE(parent.vkGetSwapchainImagesKHR(parent.hndl, hndl, &imageCount, nullptr), PFN_vkGetSwapchainImagesKHR);
	images.resize(imageCount);
	VK_VALIDATE(parent.vkGetSwapchainImagesKHR(parent.hndl, hndl, &imageCount, images.data()), PFN_vkGetSwapchainImagesKHR);

	/* Create image views. */
	for (uint32 i = 0; i < imageCount; i++)
	{
		const ImageViewCreateInfo viewCreateInfo(images[i], ImageViewType::Image2D, format);
		views.emplace_back(parent, viewCreateInfo);
	}

	/* Set the description used later to link images to render passes. */
	attachmentDesc = AttachmentDescription(createInfo.ImageFormat, ImageLayout::PresentSrcKhr, ImageLayout::PresentSrcKhr);
	//TODO: enable this once the screen gets fully rendered and not just parts of it. attachmentDesc.LoadOp = AttachmentLoadOp::DontCare;
}

Pu::Swapchain::Swapchain(Swapchain && value)
	: parent(value.parent), hndl(value.hndl), format(value.format)
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

		other.hndl = nullptr;
	}

	return *this;
}

bool Pu::Swapchain::CanCreate(const PhysicalDevice & physicalDevice, const Surface & surface, const SwapchainCreateInfo & createInfo)
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

	if (!supported) return false;

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

	if (!supported) return false;

	/* Check if image size is supported. */
	if (!capabilities.IsExtentAuto())
	{
		if (createInfo.ImageExtent < capabilities.MinImageExtent) return false;
		if (capabilities.MaxImageExtent < createInfo.ImageExtent) return false;
	}

	/* Check if image usage and transform are supported. */
	if (!_CrtEnumCheckFlag(capabilities.SupportedUsages, createInfo.ImageUsage)) return false;
	if (!_CrtEnumCheckFlag(capabilities.SupportedTransforms, createInfo.Transform)) return false;
	
	/* All checks passed so return true. */
	return true;
}

Pu::uint32 Pu::Swapchain::NextImage(const Semaphore & semaphore, uint64 timeout) const
{
	uint32 image;
	VK_VALIDATE(parent.vkAcquireNextImageKHR(parent.hndl, hndl, timeout, semaphore.hndl, nullptr, &image), PFN_vkAcquireNextImageKHR);
	return image;
}

void Pu::Swapchain::Destroy(void)
{
	if (hndl) parent.vkDestroySwapchainKHR(parent.hndl, hndl, nullptr);
}