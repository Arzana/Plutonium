#pragma once
#include "LogicalDevice.h"
#include "Content/Asset.h"

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

		/* Gets the maximum amount of mipmap layers available for a specific image size. */
		_Check_return_ static uint32 GetMaxMipLayers(_In_ Extent3D extent);
		/* Gets the maximum amount of mipmap layers available for a specific image size. */
		_Check_return_ static uint32 GetMaxMipLayers(_In_ uint32 width, _In_ uint32 height, _In_ uint32 depth);
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

		/* Gets the width of the image. */
		_Check_return_ inline uint32 GetWidth(void) const
		{
			return dimensions.Width;
		}

		/* Gets the height of the image. */
		_Check_return_ inline uint32 GetHeight(void) const
		{
			return dimensions.Height;
		}

		/* Gets the depth of the image. */
		_Check_return_ inline uint32 GetDepth(void) const
		{
			return dimensions.Depth;
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

		/* Gets the amount of mipmap levels in this image. */
		_Check_return_ inline uint32 GetMipLevels(void) const
		{
			return mipmaps;
		}

		/* Gets the amount of array layers in this image. */
		_Check_return_ inline uint32 GetArrayLayers(void) const
		{
			return layers;
		}

		/* Gets the logical device on which this image is stored. */
		_Check_return_ inline LogicalDevice& GetDevice(void)
		{
			return *parent;
		}

		/* 
		Overrides the current layout and access of the image, 
		this can be used when external calls changed the image layout.
		For instance, a subpass final layout.
		*/
		inline void OverrideState(_In_ ImageLayout newLayout, _In_ AccessFlag newAccess)
		{
			layout = newLayout;
			access = newAccess;
		}

		/* Sets a debuggable name for the image (only does something on debug mode). */
		inline void SetDebugName(_In_ const string &name) const
		{
#ifdef _DEBUG
			parent->SetDebugName(ObjectType::Image, imageHndl, name);
#else
			(void)name;
#endif
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