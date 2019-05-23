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

		/* Gets a sub-resource range spaning all sub-resources. */
		_Check_return_ ImageSubresourceRange GetFullRange(_In_ ImageAspectFlag aspect) const;
		/* Gets the amount of channels in this image. */
		_Check_return_ size_t GetChannels(void) const;

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

	protected:
		/* References the asset and returns itself. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

		/* Gets the logical device on which this image is stored. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return *parent;
		}

	private:
		friend class Swapchain;
		friend class ImageView;
		friend class CommandBuffer;
		friend class Texture;

		LogicalDevice *parent;
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
		void CanCreate(const ImageCreateInfo &info);
		void Bind(void) const;
		MemoryRequirements GetMemoryRequirements(void);
		void Destroy(void);
	};
}