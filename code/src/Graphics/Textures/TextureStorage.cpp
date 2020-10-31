#include "Graphics/Textures/TextureStorage.h"

Pu::TextureStorage::TextureStorage(Pu::Image & image, ImageViewType type)
	: Texture(nullptr, image, type)
{
#ifdef _DEBUG
	if (!image.Supports(ImageUsageFlags::Storage))
	{
		Log::Fatal("Cannot create storage texture with image that doesn't have the Storage flag!");
	}
#endif
}

Pu::TextureStorage & Pu::TextureStorage::operator=(TextureStorage && other)
{
	if (this != &other)
	{
		Destroy();
		Texture::operator=(std::move(other));

		other.Image = nullptr;
	}

	return *this;
}

void Pu::TextureStorage::Destroy(void)
{
	if (Image) delete Image;
}