#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Image::Image(LogicalDevice & device, const ImageCreateInfo & createInfo)
	: Asset(true), parent(&device), type(createInfo.ImageType), format(createInfo.Format), dimensions(createInfo.Extent),
	mipmaps(createInfo.MipLevels), usage(createInfo.Usage), layout(createInfo.InitialLayout), access(AccessFlag::None)
{
	Create(createInfo);
}

Pu::Image::Image(Image && value)
	: Asset(std::move(value)), parent(value.parent), imageHndl(value.imageHndl), memoryHndl(value.memoryHndl), type(value.type), 
	format(value.format), mipmaps(value.mipmaps), usage(value.usage), layout(value.layout), access(value.access)
{
	value.imageHndl = nullptr;
	value.memoryHndl = nullptr;
}

Pu::Image & Pu::Image::operator=(Image && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		parent = other.parent;
		imageHndl = other.imageHndl;
		memoryHndl = other.memoryHndl;
		type = other.type;
		format = other.format;
		mipmaps = other.mipmaps;
		usage = other.usage;
		layout = other.layout;
		access = other.access;

		other.imageHndl = nullptr;
		other.memoryHndl = nullptr;
	}

	return *this;
}

Pu::ImageSubresourceRange Pu::Image::GetFullRange(ImageAspectFlag aspect) const
{
	ImageSubresourceRange result(aspect);
	result.LevelCount = mipmaps;
	return result;
}

size_t Pu::Image::GetChannels(void) const
{
	switch (format)
	{
	case Format::R8_UNORM:
	case Format::R8_SNORM:
	case Format::R8_USCALED:
	case Format::R8_SSCALED:
	case Format::R8_UINT:
	case Format::R8_SINT:
	case Format::R8_SRGB:
	case Format::R16_UNORM:
	case Format::R16_SNORM:
	case Format::R16_USCALED:
	case Format::R16_SSCALED:
	case Format::R16_UINT:
	case Format::R16_SINT:
	case Format::R16_SFLOAT:
	case Format::R32_UINT:
	case Format::R32_SINT:
	case Format::R32_SFLOAT:
	case Format::R64_UINT:
	case Format::R64_SINT:
	case Format::R64_SFLOAT:
	case Format::D16_UNORM:
	case Format::X8_D24_UNORM_PACK32:
	case Format::D32_SFLOAT:
	case Format::S8_UINT:
		return 1;
	case Format::R8G8_UNORM:
	case Format::R8G8_SNORM:
	case Format::R8G8_USCALED:
	case Format::R8G8_SSCALED:
	case Format::R8G8_UINT:
	case Format::R8G8_SINT:
	case Format::R8G8_SRGB:
	case Format::R16G16_UNORM:
	case Format::R16G16_SNORM:
	case Format::R16G16_USCALED:
	case Format::R16G16_SSCALED:
	case Format::R16G16_UINT:
	case Format::R16G16_SINT:
	case Format::R16G16_SFLOAT:
	case Format::R32G32_UINT:
	case Format::R32G32_SINT:
	case Format::R32G32_SFLOAT:
	case Format::R64G64_UINT:
	case Format::R64G64_SINT:
	case Format::R64G64_SFLOAT:
	case Format::D16_UNORM_S8_UINT:
	case Format::D24_UNORM_S8_UINT:
	case Format::D32_SFLOAT_S8_UINT:
		return 2;
	case Format::R8G8B8_UNORM:
	case Format::R8G8B8_SNORM:
	case Format::R8G8B8_USCALED:
	case Format::R8G8B8_SSCALED:
	case Format::R8G8B8_UINT:
	case Format::R8G8B8_SINT:
	case Format::R8G8B8_SRGB:
	case Format::B8G8R8_UNORM:
	case Format::B8G8R8_SNORM:
	case Format::B8G8R8_USCALED:
	case Format::B8G8R8_SSCALED:
	case Format::B8G8R8_UINT:
	case Format::B8G8R8_SINT:
	case Format::B8G8R8_SRGB:
	case Format::A2R10G10B10_UNORM_PACK32:
	case Format::A2R10G10B10_SNORM_PACK32:
	case Format::A2R10G10B10_USCALED_PACK32:
	case Format::A2R10G10B10_SSCALED_PACK32:
	case Format::A2R10G10B10_UINT_PACK32:
	case Format::A2R10G10B10_SINT_PACK32:
	case Format::R16G16B16_UNORM:
	case Format::R16G16B16_SNORM:
	case Format::R16G16B16_USCALED:
	case Format::R16G16B16_SSCALED:
	case Format::R16G16B16_UINT:
	case Format::R16G16B16_SINT:
	case Format::R16G16B16_SFLOAT:
	case Format::R32G32B32_UINT:
	case Format::R32G32B32_SINT:
	case Format::R32G32B32_SFLOAT:
	case Format::R64G64B64_UINT:
	case Format::R64G64B64_SINT:
	case Format::R64G64B64_SFLOAT:
		return 3;
	case Format::R8G8B8A8_UNORM:
	case Format::R8G8B8A8_SNORM:
	case Format::R8G8B8A8_USCALED:
	case Format::R8G8B8A8_SSCALED:
	case Format::R8G8B8A8_UINT:
	case Format::R8G8B8A8_SINT:
	case Format::R8G8B8A8_SRGB:
	case Format::B8G8R8A8_UNORM:
	case Format::B8G8R8A8_SNORM:
	case Format::B8G8R8A8_USCALED:
	case Format::B8G8R8A8_SSCALED:
	case Format::B8G8R8A8_UINT:
	case Format::B8G8R8A8_SINT:
	case Format::B8G8R8A8_SRGB:
	case Format::A8B8G8R8_UNORM_PACK32:
	case Format::A8B8G8R8_SNORM_PACK32:
	case Format::A8B8G8R8_USCALED_PACK32:
	case Format::A8B8G8R8_SSCALED_PACK32:
	case Format::A8B8G8R8_UINT_PACK32:
	case Format::A8B8G8R8_SINT_PACK32:
	case Format::A8B8G8R8_SRGB_PACK32:
	case Format::A2B10G10R10_UNORM_PACK32:
	case Format::A2B10G10R10_SNORM_PACK32:
	case Format::A2B10G10R10_USCALED_PACK32:
	case Format::A2B10G10R10_SSCALED_PACK32:
	case Format::A2B10G10R10_UINT_PACK32:
	case Format::A2B10G10R10_SINT_PACK32:
	case Format::R16G16B16A16_UNORM:
	case Format::R16G16B16A16_SNORM:
	case Format::R16G16B16A16_USCALED:
	case Format::R16G16B16A16_SSCALED:
	case Format::R16G16B16A16_UINT:
	case Format::R16G16B16A16_SINT:
	case Format::R16G16B16A16_SFLOAT:
	case Format::R32G32B32A32_UINT:
	case Format::R32G32B32A32_SINT:
	case Format::R32G32B32A32_SFLOAT:
	case Format::R64G64B64A64_UINT:
	case Format::R64G64B64A64_SINT:
	case Format::R64G64B64A64_SFLOAT:
		return 4;
	default:
		return 0;
	}
}

Pu::Asset & Pu::Image::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

/* We don't allow this image to be copied as it's memory is handled by another system (like the OS). */
Pu::Image::Image(LogicalDevice & device, ImageHndl hndl, ImageType type, Format format, Extent3D extent, uint32 mipmaps, ImageUsageFlag usage, ImageLayout layout, AccessFlag access)
	: Asset(false, std::hash<ImageHndl>{}(hndl)), parent(&device), imageHndl(hndl),
	memoryHndl(nullptr), type(type), format(format), dimensions(extent), mipmaps(mipmaps), usage(usage), layout(layout), access(access)
{
#ifdef _DEBUG
	parent->SetDebugName(ObjectType::Image, hndl, u8"OS Image");
#endif

	MarkAsLoaded(false, L"OS Image");
}

void Pu::Image::Create(const ImageCreateInfo & createInfo)
{
	/* Try to throw a better error than Vulkan. */
	CanCreate(createInfo);
	MemoryPropertyFlag properties = MemoryPropertyFlag::None;

	/* Create image object. */
	VK_VALIDATE(parent->vkCreateImage(parent->hndl, &createInfo, nullptr, &imageHndl), PFN_vkCreateImage);
	SetHash(std::hash<ImageHndl>{}(imageHndl));

	/* Find the type of memory that best supports our needs. */
	uint32 typeIdx;
	const MemoryRequirements requirements = GetMemoryRequirements();
	if (parent->parent->GetBestMemoryType(requirements.MemoryTypeBits, properties, false, typeIdx))
	{
		/* Allocate the image's data. */
		const MemoryAllocateInfo allocateInfo(requirements.Size, typeIdx);
		VK_VALIDATE(parent->vkAllocateMemory(parent->hndl, &allocateInfo, nullptr, &memoryHndl), PFN_vkAllocateMemory);

		/* Bind the memory to the image. */
		Bind();
	}
	else Log::Fatal("Unable to allocate memory for image!");
}

void Pu::Image::CanCreate(const ImageCreateInfo & info)
{
	bool log = false;
	string error = "Unable to create image";

	const FormatProperties formatProps = parent->parent->GetFormatProperties(info.Format);
	if (!_CrtEnumCheckFlag(info.Tiling == ImageTiling::Optimal ? formatProps.OptimalTilingFeatures : formatProps.LinearTilingFeatures, FormatFeatureFlag::SampledImage))
	{
		log = true;
		error += ", cannot create sampled image with specified format";
	}

	/* Don't check for the format properties if we've already failed. */
	if (!log)
	{
		const ImageFormatProperties imgProps = parent->parent->GetImageFormatProperties(info);
		if (imgProps.MaxArrayLayers < info.ArrayLayers)
		{
			log = true;
			error += ", too many array layers";
		}
		if (imgProps.MaxExtent < info.Extent)
		{
			log = true;
			error += ", image size is too big";
		}
		if (imgProps.MaxMipLevels < info.MipLevels)
		{
			log = true;
			error += ", too many mip levels";
		}
	}

	if (log) Log::Fatal((error += '!').c_str());
}

void Pu::Image::Bind(void) const
{
	// TODO: We should not allocate a memory object for ever image but rather make one big one and set a proper offset to increase cache hits.
	VK_VALIDATE(parent->vkBindImageMemory(parent->hndl, imageHndl, memoryHndl, 0), PFN_vkBindImageMemory);
}

Pu::MemoryRequirements Pu::Image::GetMemoryRequirements(void)
{
	MemoryRequirements result;
	parent->vkGetImageMemoryRequirements(parent->hndl, imageHndl, &result);
	return result;
}

void Pu::Image::Destroy(void)
{
	/* We don't need to destroy swapchain images so make sure both are set for a user created image. */
	if (memoryHndl && imageHndl)
	{
		parent->vkDestroyImage(parent->hndl, imageHndl, nullptr);
		parent->vkFreeMemory(parent->hndl, memoryHndl, nullptr);
	}
}