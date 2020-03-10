#include "Graphics/Vulkan/DescriptorSetBase.h" 
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorSetBase::DescriptorSetBase(DescriptorPool & pool)
	: Pool(&pool)
{}

void Pu::DescriptorSetBase::Write(DescriptorSetHndl hndl, const DescriptorSetLayout & layout, DeviceSize offset)
{
	/*
	Preallocate the required buffers (required, otherwise the vector pointer will be invalid)
	and fill them with the required data.
	*/
	vector<DescriptorBufferInfo> bufferInfos;
	bufferInfos.reserve(layout.ranges.size());
	vector<WriteDescriptorSet> writes;
	writes.reserve(layout.ranges.size());

	for (const auto&[binding, range] : layout.ranges)
	{
		bufferInfos.emplace_back(Pool->buffer->bufferHndl, offset + range.first, range.second);
		writes.emplace_back(hndl, binding, bufferInfos.back());
	}

	WriteDescriptors(writes);
}

void Pu::DescriptorSetBase::Write(DescriptorSetHndl hndl, uint32 set, const Descriptor & descriptor, const TextureInput & input)
{
	WriteInput(hndl, set, descriptor, input.view->hndl);
}

void Pu::DescriptorSetBase::Write(DescriptorSetHndl hndl, uint32 set, const Descriptor & descriptor, const DepthBuffer & input)
{
	WriteInput(hndl, set, descriptor, input.GetView().hndl);
}

void Pu::DescriptorSetBase::Write(DescriptorSetHndl hndl, uint32 set, const Descriptor & descriptor, const Texture & texture)
{
#ifdef _DEBUG
	ValidateDescriptor(descriptor, set, DescriptorType::CombinedImageSampler);
#endif

	const DescriptorImageInfo info{ texture.Sampler->hndl, texture.view->hndl };
	const WriteDescriptorSet write{ hndl, descriptor.layoutBinding.Binding, info };
	WriteDescriptors({ write });
}

void Pu::DescriptorSetBase::ValidateDescriptor(const Descriptor & descriptor, uint32 set, DescriptorType type)
{
	/* Make sure that the set matches. */
	if (descriptor.GetSet() != set)
	{
		Log::Fatal("Cannot write descriptor '%s' from set %u to set %u!",
			descriptor.GetInfo().Name.c_str(),
			descriptor.GetSet(), set);
	}

	/* Make sure that this type is accepted. */
	if (descriptor.GetType() != type)
	{
		Log::Fatal("Cannot write descriptor '%s'(%s) as a %s descriptor!",
			descriptor.GetInfo().Name.c_str(),
			to_string(descriptor.GetType()),
			to_string(type));
	}
}

void Pu::DescriptorSetBase::WriteInput(DescriptorSetHndl setHndl, uint32 set, const Descriptor & descriptor, ImageViewHndl viewHndl)
{
#ifdef _DEBUG
	ValidateDescriptor(descriptor, set, DescriptorType::InputAttachment);
#endif

	/* An input attachment descriptor doesn't have a sampler (because the samples are fragment local). */
	const DescriptorImageInfo info{ nullptr, viewHndl };
	WriteDescriptorSet write{ setHndl, descriptor.layoutBinding.Binding, info };
	write.DescriptorType = DescriptorType::InputAttachment;
	WriteDescriptors({ write });
}

void Pu::DescriptorSetBase::WriteDescriptors(const vector<WriteDescriptorSet>& writes)
{
	Pool->device->vkUpdateDescriptorSets(Pool->device->hndl, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}