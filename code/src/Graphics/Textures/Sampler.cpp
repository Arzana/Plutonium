#include "Graphics/Textures/Sampler.h"

Pu::Sampler::Sampler(LogicalDevice & device, const SamplerCreateInfo & createInfo)
	: parent(device)
{
	VK_VALIDATE(parent.vkCreateSampler(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateSampler);
}

Pu::Sampler::Sampler(Sampler && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::Sampler & Pu::Sampler::operator=(Sampler && other)
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

void Pu::Sampler::Destroy(void)
{
	if (hndl) parent.vkDestroySampler(parent.hndl, hndl, nullptr);
}