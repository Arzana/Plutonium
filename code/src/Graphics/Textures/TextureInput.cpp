#include "Graphics/Textures/TextureInput.h"

Pu::TextureInput::TextureInput(Pu::Image & image, ImageViewType type)
	: Texture(nullptr, image, type)
{
#ifdef _DEBUG
	if (!image.Supports(ImageUsageFlags::InputAttachment))
	{
		Log::Fatal("Cannot create input texture wity image that doesn't have the InputAttachment flag!");
	}
#endif
}

Pu::TextureInput & Pu::TextureInput::operator=(TextureInput && other)
{
	if (this != &other)
	{
		Destroy();
		Texture::operator=(std::move(other));

		other.Image = nullptr;
	}

	return *this;
}

void Pu::TextureInput::Destroy(void)
{
	if (Image) delete Image;
}