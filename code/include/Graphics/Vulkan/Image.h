#pragma once
#include "LogicalDevice.h"

namespace Pu
{
	/* Defines a Vulkan image. */
	class Image
	{
	public:
		/* Initializes a new instance of an image. */
		Image(_In_ LogicalDevice &device, _In_ const ImageCreateInfo &createInfo);
		Image(_In_ const Image&) = delete;
		/* Move constructor. */
		Image(_In_ Image &&value);
		/* Releases the image. */
		virtual ~Image(void)
		{
			Destroy();
		}

		_Check_return_ Image& operator =(_In_ const Image&) = delete;
		/* Move assignment. */
		_Check_return_ Image& operator =(_In_ Image &&other);

		/* Gets the size of the image. */
		_Check_return_ inline Extent3D GetExtent(void) const
		{
			return dimensions;
		}

	private:
		friend class Swapchain;
		friend class ImageView;
		friend class CommandBuffer;

		LogicalDevice &parent;
		ImageHndl imageHndl;
		DeviceMemoryHndl memoryHndl;

		ImageType type;
		Format format;
		Extent3D dimensions;
		uint32 mipmaps;
		ImageUsageFlag usage;
		mutable ImageLayout layout;
		mutable AccessFlag access;

		Image(LogicalDevice &device, ImageHndl hndl, ImageType type, Format format, Extent3D extent, uint32 mipmaps, ImageUsageFlag usage, ImageLayout layout, AccessFlag access);

		void Create(const ImageCreateInfo &createInfo);
		void Bind(void) const;
		MemoryRequirements GetMemoryRequirements(void);
		void Destroy(void);
	};
}