#include "Graphics/Textures/Sampler.h"

Pu::Sampler::Sampler(LogicalDevice & device, const SamplerCreateInfo & createInfo)
	: Asset(true, CreateHash(createInfo)), parent(&device), 
	magFilter(createInfo.MagFilter), minFilter(createInfo.MinFilter), mipmapMode(createInfo.MipmapMode),
	uMode(createInfo.AddressModeU), vMode(createInfo.AddressModeV), wMode(createInfo.AddressModeW),
	loDBias(createInfo.MipLodBias), maxAnisotropy(createInfo.MaxAnisotropy), minLoD(createInfo.MinLoD), maxLoD(createInfo.MaxLoD),
	anisotropy(createInfo.AnisotropyEnable), compare(createInfo.CompareModeEnable), unnormalizedCoordinates(createInfo.UnnormalizedCoordinates),
	cmpOp(createInfo.CompareOp), clr(createInfo.BorderColor)
{
	VK_VALIDATE(parent->vkCreateSampler(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateSampler);
	MarkAsLoaded(false, L"Sampler");	// Default to not loaded via load, loader will override this.
}

Pu::Sampler::Sampler(Sampler && value)
	: Asset(std::move(value)), parent(value.parent),
	hndl(value.hndl), uMode(value.uMode), vMode(value.vMode), wMode(value.wMode),
	magFilter(value.magFilter), minFilter(value.minFilter), mipmapMode(value.mipmapMode),
	loDBias(value.loDBias), maxAnisotropy(value.maxAnisotropy), minLoD(value.minLoD), maxLoD(value.maxLoD),
	anisotropy(value.anisotropy), compare(value.compare), unnormalizedCoordinates(value.unnormalizedCoordinates),
	cmpOp(value.cmpOp), clr(value.clr)
{
	value.hndl = nullptr;
}

Pu::Sampler & Pu::Sampler::operator=(Sampler && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		parent = other.parent;
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

bool Pu::Sampler::operator==(const Sampler & other) const
{
	/* Samplers are still functionally equal if they have anisotropy disabled but different max values for it. */
	if (anisotropy && other.anisotropy)
	{
		if (maxAnisotropy != other.maxAnisotropy) return false;
	}
	else if (anisotropy != other.anisotropy) return false;

	/* Samplers are still functionally equal if they have compare disables but different modes for it. */
	if (compare && other.compare)
	{
		if (cmpOp != other.cmpOp) return false;
	}
	else if (compare != other.compare) return false;

	/* Samplers are still functionally equal if they have no clamp to border address modes but different border colors. */
	if ((uMode == SamplerAddressMode::ClampToBorder && other.uMode == SamplerAddressMode::ClampToBorder) ||
		(vMode == SamplerAddressMode::ClampToBorder && other.vMode == SamplerAddressMode::ClampToBorder) ||
		(wMode == SamplerAddressMode::ClampToBorder && other.wMode == SamplerAddressMode::ClampToBorder))
	{
		if (clr != other.clr) return false;
	}

	/* Check for the required values. */
	return other.magFilter == magFilter
		&& other.minFilter == minFilter
		&& other.mipmapMode == mipmapMode
		&& other.uMode == uMode
		&& other.vMode == vMode
		&& other.wMode == wMode
		&& other.loDBias == loDBias
		&& other.minLoD == minLoD
		&& other.maxLoD == maxLoD
		&& other.unnormalizedCoordinates == unnormalizedCoordinates;
}

Pu::Asset & Pu::Sampler::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

size_t Pu::Sampler::CreateHash(const SamplerCreateInfo & info)
{
	size_t hash = std::hash<Filter>{}(info.MagFilter);
	hash = std::hash_combine(hash, info.MinFilter);
	hash = std::hash_combine(hash, info.MipmapMode);
	hash = std::hash_combine(hash, info.AddressModeU);
	hash = std::hash_combine(hash, info.AddressModeV);
	hash = std::hash_combine(hash, info.AddressModeW);
	hash = std::hash_combine(hash, info.MipLodBias);
	hash = std::hash_combine(hash, info.AnisotropyEnable);
	hash = std::hash_combine(hash, info.MaxAnisotropy);
	hash = std::hash_combine(hash, info.CompareModeEnable);
	hash = std::hash_combine(hash, info.MinLoD);
	hash = std::hash_combine(hash, info.MaxLoD);
	hash = std::hash_combine(hash, info.BorderColor);
	return std::hash_combine(hash, info.UnnormalizedCoordinates);
}

void Pu::Sampler::Destroy(void)
{
	if (hndl) parent->vkDestroySampler(parent->hndl, hndl, nullptr);
}