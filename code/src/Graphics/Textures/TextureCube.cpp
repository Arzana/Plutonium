#include "Graphics/Textures/TextureCube.h"

Pu::TextureCube::TextureCube(Pu::Image & image, Pu::Sampler & sampler)
	: Texture(sampler, image, ImageViewType::ImageCube)
{}

Pu::TextureCube::TextureCube(TextureCube && value)
	: Texture(std::move(value))
{}

Pu::TextureCube & Pu::TextureCube::operator=(TextureCube && other)
{
	if (this != &other) Texture::operator=(std::move(other));
	return *this;
}