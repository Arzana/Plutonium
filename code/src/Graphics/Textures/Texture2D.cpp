#include "Graphics/Textures/Texture2D.h"

Pu::Texture2D::Texture2D(LogicalDevice & device, Sampler & sampler, Format format, Extent2D extent, ImageUsageFlag usage)
	: Texture2D(device, sampler, format, extent, usage, DefaultMipLevels, 1, SampleCountFlag::Pixel1Bit)
{}

Pu::Texture2D::Texture2D(LogicalDevice & device, Sampler & sampler, Format format, Extent2D extent, ImageUsageFlag usage, uint32 mipLevels, uint32 arrayLayers, SampleCountFlag samples)
	: Texture(device, sampler, ImageCreateInfo(ImageType::Image2D, format, Extent3D(extent, 1), mipLevels, arrayLayers, samples, usage))
{}

Pu::Texture2D::Texture2D(Texture2D && value)
	: Texture(std::move(value))
{}

Pu::Texture2D & Pu::Texture2D::operator=(Texture && other)
{
	if (this != &other) Texture::operator=(std::move(other));

	return *this;
}
