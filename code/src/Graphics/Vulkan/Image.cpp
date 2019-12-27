#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Image::Image(LogicalDevice & device, const ImageCreateInfo & createInfo)
	: Asset(true), parent(&device), type(createInfo.ImageType), format(createInfo.Format), dimensions(createInfo.Extent),
	mipmaps(createInfo.MipLevels), usage(createInfo.Usage), layout(createInfo.InitialLayout), access(AccessFlag::None),
	layers(createInfo.ArrayLayers)
{
	Create(createInfo);
}

Pu::Image::Image(Image && value)
	: Asset(std::move(value)), parent(value.parent), imageHndl(value.imageHndl), memoryHndl(value.memoryHndl), type(value.type), 
	format(value.format), mipmaps(value.mipmaps), usage(value.usage), layout(value.layout), access(value.access), 
	dimensions(value.dimensions), layers(value.layers)
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
		dimensions = other.dimensions;
		layers = other.layers;

		other.imageHndl = nullptr;
		other.memoryHndl = nullptr;
	}

	return *this;
}

Pu::uint32 Pu::Image::GetMaxMipLayers(Extent3D extent)
{
	return static_cast<uint32>(floor(log2(max(extent.Width, extent.Height, extent.Depth))) + 1);
}

Pu::uint32 Pu::Image::GetMaxMipLayers(uint32 width, uint32 height, uint32 depth)
{
	return static_cast<uint32>(floor(log2(max(width, height, depth))) + 1);
}

Pu::DeviceSize Pu::Image::GetLazyMemory(void) const
{
	if (memoryHndl)
	{
		DeviceSize result;
		parent->vkGetDeviceMemoryCommitment(parent->hndl, memoryHndl, &result);
		return result;
	}
	else return 0;
}

Pu::ImageSubresourceRange Pu::Image::GetFullRange(ImageAspectFlag aspect) const
{
	ImageSubresourceRange result(aspect);
	result.LevelCount = mipmaps;
	result.LayerCount = layers;
	return result;
}

Pu::Asset & Pu::Image::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

/* We don't allow this image to be copied as it's memory is handled by another system (like the OS). */
Pu::Image::Image(LogicalDevice & device, ImageHndl hndl, ImageType type, Format format, Extent3D extent, uint32 mipmaps, ImageUsageFlag usage, ImageLayout layout, AccessFlag access)
	: Asset(false, std::hash<ImageHndl>{}(hndl)), parent(&device), imageHndl(hndl), layers(1),
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
		error += ", cannot create sampled image with '";
		error += to_string(info.Format);
		error += "' format";
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