#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorSet::DescriptorSet(DescriptorPool & pool, uint32 subpass, const DescriptorSetLayout & setLayout)
	: DescriptorSetBase(pool), set(setLayout.GetSet()), subscribe(setLayout.HasUniformBufferMemory())
{
	/* Allocate the new buffer and get the base offset into the pools buffer. */
	baseOffset = pool.Alloc(subpass, setLayout, &hndl);

	/* Write the descriptors in the layout to the parent pool if it is a uniform block. */
	if (subscribe)
	{
		DescriptorSetBase::Write(hndl, setLayout, baseOffset);
		pool.OnStage.Add(*this, &DescriptorSet::StageInternal);
	}
}

Pu::DescriptorSet::DescriptorSet(DescriptorSet && value)
	: DescriptorSetBase(std::move(value)), hndl(value.hndl), set(value.set),
	baseOffset(value.baseOffset), subscribe(value.subscribe)
{
	value.hndl = nullptr;
	if (subscribe) Pool->OnStage.Add(*this, &DescriptorSet::StageInternal);
}

Pu::DescriptorSet & Pu::DescriptorSet::operator=(DescriptorSet && other)
{
	if (this != &other)
	{
		DescriptorSetBase::operator=(std::move(other));
		Destroy();

		hndl = other.hndl;
		set = other.set;
		baseOffset = other.baseOffset;
		subscribe = other.subscribe;

		if (subscribe) Pool->OnStage.Add(*this, &DescriptorSet::StageInternal);
		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const TextureInput & input)
{
	DescriptorSetBase::Write(hndl, set, descriptor, input);
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const DepthBuffer & input)
{
	DescriptorSetBase::Write(hndl, set, descriptor, input);
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const Texture & texture)
{
	DescriptorSetBase::Write(hndl, set, descriptor, texture);
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const ImageView & image)
{
	DescriptorSetBase::Write(hndl, set, descriptor, image);
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const Buffer & buffer)
{
	DescriptorSetBase::Write(hndl, set, descriptor, buffer);
}

void Pu::DescriptorSet::StageInternal(DescriptorPool &, byte * destination)
{
	Stage(destination + baseOffset);
}

void Pu::DescriptorSet::Free(void)
{
	Destroy();
	hndl = nullptr;
}

void Pu::DescriptorSet::Destroy(void)
{
	if (hndl) Pool->Free(hndl);

	if (subscribe)
	{
		subscribe = false;
		Pool->OnStage.Remove(*this, &DescriptorSet::StageInternal);
	}
}