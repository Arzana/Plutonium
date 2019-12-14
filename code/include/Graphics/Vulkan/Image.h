#pragma once
#include "LogicalDevice.h"
#include "Content/Asset.h"
#include "Graphics/Textures/ImageSaveFormats.h"

namespace Pu
{
	/* Defines a Vulkan image. */
	class Image
		: public Asset
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

		/* Gets the amount of lazily-allocated bytes that are committed for the image. */
		_Check_return_ DeviceSize GetLazyMemory(void) const;
		/* Gets a sub-resource range spaning all sub-resources. */
		_Check_return_ ImageSubresourceRange GetFullRange(_In_ ImageAspectFlag aspect) const;

		/* Gets the amount of channels in this image. */
		_Check_return_ inline size_t GetChannels(void) const
		{
			return format_channels(format);
		}

		/* Gets the size of the image. */
		_Check_return_ inline Extent3D GetExtent(void) const
		{
			return dimensions;
		}

		/* Gets the format of the image. */
		_Check_return_ inline Format GetFormat(void) const
		{
			return format;
		}

		/* Gets the current memory layout of the image. */
		_Check_return_ inline ImageLayout GetLayout(void) const
		{
			return layout;
		}

		/* Gets the logical device on which this image is stored. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return *parent;
		}

	protected:
		/* References the asset and returns itself. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

	private:
		friend class Swapchain;
		friend class ImageView;
		friend class CommandBuffer;

		LogicalDevice *parent;
		ImageHndl imageHndl;
		DeviceMemoryHndl memoryHndl;

		ImageType type;
		Format format;
		Extent3D dimensions;
		uint32 mipmaps, layers;
		ImageUsageFlag usage;
		mutable ImageLayout layout;
		mutable AccessFlag access;

		Image(LogicalDevice &device, ImageHndl hndl, ImageType type, Format format, Extent3D extent, uint32 mipmaps, ImageUsageFlag usage, ImageLayout layout, AccessFlag access);

		void Create(const ImageCreateInfo &createInfo);
		void CanCreate(const ImageCreateInfo &info);
		void Bind(void) const;
		MemoryRequirements GetMemoryRequirements(void);
		void Destroy(void);
	};
}