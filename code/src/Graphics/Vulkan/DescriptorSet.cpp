#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Resources/DynamicBuffer.h"

Pu::DescriptorSet::DescriptorSet(DescriptorPool & pool, uint32 subpass, const DescriptorSetLayout & setLayout)
	: pool(&pool), set(setLayout.set)
{
	baseOffset = pool.Alloc(subpass, setLayout, &hndl);
	if (subscribe = setLayout.HasUniformBufferMemory())
	{
		WriteBuffer(setLayout);
		pool.OnStage.Add(*this, &DescriptorSet::StageInternal);
	}
}

Pu::DescriptorSet::DescriptorSet(DescriptorSet && value)
	: hndl(value.hndl), pool(value.pool), set(value.set),
	baseOffset(value.baseOffset), subscribe(value.subscribe)
{
	value.hndl = nullptr;
	if (subscribe) pool->OnStage.Add(*this, &DescriptorSet::StageInternal);
}

Pu::DescriptorSet & Pu::DescriptorSet::operator=(DescriptorSet && other)
{
	if (this != &other)
	{
		Destroy();

		hndl = other.hndl;
		pool = other.pool;
		set = other.set;
		baseOffset = other.baseOffset;
		subscribe = other.subscribe;

		if (subscribe) pool->OnStage.Add(*this, &DescriptorSet::StageInternal);
		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const TextureInput & input)
{
	ValidateDescriptor(descriptor, DescriptorType::InputAttachment);

	/* An input attachment descriptor doesn't have a sampler (because the samples are fragment local). */
	const DescriptorImageInfo info{ nullptr, input.view->hndl };
	WriteDescriptorSet write{ hndl, descriptor.layoutBinding.Binding, info };
	write.DescriptorType = DescriptorType::InputAttachment;
	WriteDescriptors({ write });
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const Texture & texture)
{
	ValidateDescriptor(descriptor, DescriptorType::CombinedImageSampler);

	const DescriptorImageInfo info{ texture.Sampler->hndl, texture.view->hndl };
	const WriteDescriptorSet write{ hndl, descriptor.layoutBinding.Binding, info };
	WriteDescriptors({ write });
}

void Pu::DescriptorSet::WriteBuffer(const DescriptorSetLayout & layout)
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
		bufferInfos.emplace_back(pool->buffer->bufferHndl, baseOffset + range.first, range.second);
		writes.emplace_back(hndl, binding, bufferInfos.back());
	}

	WriteDescriptors(writes);
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

void Pu::DescriptorSet::ValidateDescriptor(const Descriptor & descriptor, DescriptorType type) const
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

void Pu::DescriptorSet::WriteDescriptors(const vector<WriteDescriptorSet>& writes)
{
	pool->device->vkUpdateDescriptorSets(pool->device->hndl, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}

void Pu::DescriptorSet::Destroy(void)
{
	if (hndl)
	{
		pool->Free(hndl);
		if (subscribe) pool->OnStage.Remove(*this, &DescriptorSet::StageInternal);
	}
}