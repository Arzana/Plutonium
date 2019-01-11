#include "Graphics/Textures/Texture.h"

Pu::Texture::Texture(Texture && value)
	: Image(std::move(value)), sampler(value.sampler), view(value.view)
{
	value.view = nullptr;
}

Pu::Texture & Pu::Texture::operator=(Texture && other)
{
	if (this != &other)
	{
		Image::operator=(std::move(other));

		Destroy();
		sampler = std::move(other.sampler);
		view = other.view;

		other.view = nullptr;
	}

	return *this;
}

Pu::Texture::Texture(LogicalDevice & device, Sampler & sampler, const ImageCreateInfo & createInfo)
	: Image(device, createInfo), sampler(sampler)
{
	view = new ImageView(*this);
}

void Pu::Texture::Destroy(void)
{
	if (view) delete view;
}