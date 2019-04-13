#include "Graphics/Vulkan/ImageView.h"

using namespace Pu;

/* Not all codepaths return a value, Log::Fatal will always throw. */
#pragma warning(push)
#pragma warning(disable:4715)
static ImageViewType imgTypeToViewType(ImageType type)
{
	switch (type)
	{
	case ImageType::Image1D:
		return ImageViewType::Image1D;
	case ImageType::Image2D:
		return ImageViewType::Image2D;
	case ImageType::Image3D:
		return ImageViewType::Image3D;
	}

	Log::Fatal("Unknown image type passed!");
}
#pragma warning(pop)

Pu::ImageView::ImageView(const Image & image, ImageAspectFlag aspect)
	: parent(image.parent)
{
	const ImageViewCreateInfo createInfo(image.imageHndl, imgTypeToViewType(image.type), image.format, aspect);
	VK_VALIDATE(parent.vkCreateImageView(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateImageView);
}

Pu::ImageView::ImageView(ImageView && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::ImageView & Pu::ImageView::operator=(ImageView && other)
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

void Pu::ImageView::Destroy(void)
{
	if (hndl) parent.vkDestroyImageView(parent.hndl, hndl, nullptr);
}