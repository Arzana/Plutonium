#include "Graphics/Textures/TextureInput.h"

Pu::TextureInput::TextureInput(Pu::Image & image, ImageViewType type)
	: Texture(nullptr, image, type)
{}

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