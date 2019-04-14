#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

Pu::DescriptorSet::DescriptorSet(DescriptorSet && value)
	: parent(value.parent), hndl(value.hndl), set(value.set)
{
	value.hndl = nullptr;
}

Pu::DescriptorSet & Pu::DescriptorSet::operator=(DescriptorSet && other)
{
	if (this != &other)
	{
		Free();

		parent = std::move(other.parent);
		hndl = other.hndl;
		set = other.set;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Uniform & uniform, const Texture & texture)
{
	/* Check if the descriptor type is correct. */
	if (uniform.layoutBinding.DescriptorType != DescriptorType::CombinedImageSampler) Log::Fatal("Cannot update descriptor with image on non-image uniform!");

	/* Write the descriptor. */
	DescriptorImageInfo info(texture.Sampler.hndl, texture.view->hndl);
	WriteDescriptorSet write(hndl, uniform.layoutBinding.Binding, info);
	WriteDescriptor({ write });
}

void Pu::DescriptorSet::Write(const vector<const Uniform*>& uniforms, const Buffer & buffer)
{
	uint32 binding = 0;
	DeviceSize size = 0;

	/* Loop through all uniforms to get the size and check the uniforms. */
	for (size_t i = 0; i < uniforms.size(); i++)
	{
		const Uniform &cur = *uniforms[i];

		/* Check if the descriptor type is correct. */
		if (cur.layoutBinding.DescriptorType != DescriptorType::UniformBuffer) Log::Fatal("Cannot update descriptor with buffer on non-buffer uniform!");

		/* Make sure all the descriptors are in the same uniform buffer. */
		if (!i) binding = cur.layoutBinding.Binding;
		else if (cur.layoutBinding.Binding != binding) Log::Fatal("Cannot pass uniforms from seperate bindings to descriptor write!");

		size += cur.GetSize();
	}

	DescriptorBufferInfo info(buffer.bufferHndl, 0, size);
	WriteDescriptorSet write(hndl, binding, info);
	WriteDescriptor({ write });
}

Pu::DescriptorSet::DescriptorSet(DescriptorPool & pool, DescriptorSetHndl hndl, uint32 set)
	: parent(pool), hndl(hndl), set(set)
{}

void Pu::DescriptorSet::WriteDescriptor(const vector<WriteDescriptorSet> & writes)
{
	parent.parent.parent.vkUpdateDescriptorSets(parent.parent.parent.hndl, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}

void Pu::DescriptorSet::Free(void)
{
	if (hndl) parent.FreeSet(hndl);
}