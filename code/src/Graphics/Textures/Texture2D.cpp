#include "Graphics/Textures/Texture2D.h"

Pu::Texture2D::Texture2D(Pu::Image & image, Pu::Sampler & sampler)
	: Texture(sampler, image)
{}

Pu::Texture2D::Texture2D(Texture2D && value)
	: Texture(std::move(value))
{}

Pu::Texture2D & Pu::Texture2D::operator=(Texture && other)
{
	if (this != &other) Texture::operator=(std::move(other));
	return *this;
}
