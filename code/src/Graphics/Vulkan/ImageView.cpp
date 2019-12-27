#include "Graphics/Vulkan/ImageView.h"

using namespace Pu;

Pu::ImageView::ImageView(const Image & image, ImageViewType type, ImageAspectFlag aspect)
	: parent(image.parent)
{
	ImageViewCreateInfo createInfo(image.imageHndl, type, image.format, aspect);
	createInfo.SubresourceRange.LayerCount = image.layers;
	createInfo.SubresourceRange.LevelCount = RemainingMipLevels;

	VK_VALIDATE(parent->vkCreateImageView(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateImageView);
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
	if (hndl) parent->vkDestroyImageView(parent->hndl, hndl, nullptr);
}