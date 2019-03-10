#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/PhysicalDevice.h"

Pu::Image::Image(LogicalDevice & device, const ImageCreateInfo & createInfo)
	: Asset(true), parent(device), type(createInfo.ImageType), format(createInfo.Format), dimensions(createInfo.Extent),
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
		parent = std::move(other.parent);
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

Pu::ImageSubresourceRange Pu::Image::GetFullRange(void) const
{
	ImageSubresourceRange result(ImageAspectFlag::Color);
	result.LevelCount = mipmaps;
	return result;
}

Pu::Asset & Pu::Image::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

/* We don't allow this image to be copied as it's memory is handled by another system (like the OS). */
Pu::Image::Image(LogicalDevice & device, ImageHndl hndl, ImageType type, Format format, Extent3D extent, uint32 mipmaps, ImageUsageFlag usage, ImageLayout layout, AccessFlag access)
	: Asset(false, std::hash<ImageHndl>{}(hndl)), parent(device), imageHndl(hndl),
	memoryHndl(nullptr), type(type), format(format), dimensions(extent), mipmaps(mipmaps), usage(usage), layout(layout), access(access)
{
	MarkAsLoaded(false);
}

void Pu::Image::Create(const ImageCreateInfo & createInfo)
{
	MemoryPropertyFlag properties = MemoryPropertyFlag::None;

	/* Create image object. */
	VK_VALIDATE(parent.vkCreateImage(parent.hndl, &createInfo, nullptr, &imageHndl), PFN_vkCreateImage);
	SetHash(std::hash<ImageHndl>{}(imageHndl));

	/* Find the type of memory that best supports our needs. */
	uint32 typeIdx;
	const MemoryRequirements requirements = GetMemoryRequirements();
	if (parent.parent.GetBestMemoryType(requirements.MemoryTypeBits, properties, false, typeIdx))
	{
		/* Allocate the image's data. */
		const MemoryAllocateInfo allocateInfo(requirements.Size, typeIdx);
		VK_VALIDATE(parent.vkAllocateMemory(parent.hndl, &allocateInfo, nullptr, &memoryHndl), PFN_vkAllocateMemory);

		/* Bind the memory to the image. */
		Bind();
	}
	else Log::Fatal("Unable to allocate memory for image!");
}

void Pu::Image::Bind(void) const
{
	// TODO: We should not allocate a memory object for ever image but rather make one big one and set a proper offset to increase cache hits.
	VK_VALIDATE(parent.vkBindImageMemory(parent.hndl, imageHndl, memoryHndl, 0), PFN_vkBindImageMemory);
}

Pu::MemoryRequirements Pu::Image::GetMemoryRequirements(void)
{
	MemoryRequirements result;
	parent.vkGetImageMemoryRequirements(parent.hndl, imageHndl, &result);
	return result;
}

void Pu::Image::Destroy(void)
{
	/* We don't need to destroy swapchain images so make sure both are set for a user created image. */
	if (memoryHndl && imageHndl)
	{
		parent.vkDestroyImage(parent.hndl, imageHndl, nullptr);
		parent.vkFreeMemory(parent.hndl, memoryHndl, nullptr);
	}
}