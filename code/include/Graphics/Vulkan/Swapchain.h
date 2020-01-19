#pragma once
#include "Image.h"
#include "ImageView.h"
#include "Surface.h"
#include "Semaphore.h"

namespace Pu
{
	/* Defines a swapchain. */
	class Swapchain
	{
	public:
		/* Initializes a new instance of a swapchain for a specific device, physical device and for a specific surface with specific arguments. */
		Swapchain(_In_ LogicalDevice &device, _In_ const Surface &surface, _In_ const SwapchainCreateInfo &createInfo);
		Swapchain(_In_ const Swapchain&) = delete;
		/* Move constructor. */
		Swapchain(_In_ Swapchain &&value);
		/* Destroys the swapchain. */
		~Swapchain(void)
		{
			Destroy();
		}

		_Check_return_ Swapchain& operator =(_In_ const Swapchain&) = delete;
		/* Move assignment. */
		_Check_return_ Swapchain& operator =(_In_ Swapchain &&other);

		/* Checks whether the specified physical device and surface support the specified swapchain. */
		_Check_return_ static bool CanCreate(_In_ const PhysicalDevice &physicalDevice, _In_ const Surface &surface, _In_ const SwapchainCreateInfo &createInfo);

		/* Acquires the next image index available in the swapchain. */
		_Check_return_ uint32 NextImage(_In_ const Semaphore &semaphore, _In_opt_ uint64 timeout = 16666666) const;

		/* Gets the attachment information for one of the swapchain images. */
		_Check_return_ inline const AttachmentDescription& GetAttachmentDescription(void) const
		{
			return attachmentDesc;
		}

		/* Gets the image handle at the specified index. */
		_Check_return_ inline const Image& GetImage(_In_ uint32 index) const
		{
			return images.at(index);
		}

		/* Gets the image view at the specified index. */
		_Check_return_ inline const ImageView& GetImageView(_In_ uint32 index) const
		{
			return views.at(index);
		}

		/* Gets the underlying format of the swapchain images. */
		_Check_return_ inline Format GetImageFormat(void) const
		{
			return format.Format;
		}

		/* Gets the color space of the swapchain images. */
		_Check_return_ inline ColorSpace GetColorSpace(void) const
		{
			return format.ColorSpace;
		}

		/* Gets the surface format of the swapchain images. */
		_Check_return_ inline const SurfaceFormat& GetFormat(void) const
		{
			return format;
		}

		/* Gets whether the swapchain image format is compatible with native HDR. */
		_Check_return_ inline bool IsNativeHDR(void) const
		{
			return format_component_format(format.Format) == NumericFormat::SFloat;
		}

		/* Gets the amount of images created by the operating system. */
		_Check_return_ inline size_t GetImageCount(void) const
		{
			return images.size();
		}

	private:
		friend class Queue;
		friend class GameWindow;

		LogicalDevice *parent;
		SwapchainHndl hndl;
		vector<Image> images;
		vector<ImageView> views;
		SurfaceFormat format;
		AttachmentDescription attachmentDesc;

		static bool CanCreateInternal(const PhysicalDevice &physicalDevice, const Surface &surface, const SwapchainCreateInfo &createInfo, bool raise);
		
		void AquireImages(const SwapchainCreateInfo &createInfo);
		void Destroy(void);
	};
}