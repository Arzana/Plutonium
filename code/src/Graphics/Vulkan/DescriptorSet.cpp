#include "Graphics/Vulkan/DescriptorSet.h"

Pu::DescriptorSet::DescriptorSet(const DescriptorPool & pool, const Pipeline & pipeline, uint32 set)
	: pool(&pool), set(set)
{
	/* Sets handles are always incremental. */
	baseOffset = pool.Alloc(pipeline.setHndls.at(set), &hndl);
}

Pu::DescriptorSet::DescriptorSet(DescriptorSet && value)
	: hndl(value.hndl), pool(value.pool), set(value.set)
{
	value.hndl = nullptr;
}

Pu::DescriptorSet & Pu::DescriptorSet::operator=(DescriptorSet && other)
{
	if (this != &other)
	{
		Destroy();

		hndl = other.hndl;
		pool = other.pool;
		set = other.set;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const TextureInput & input)
{
	ValidateDescriptor(descriptor, DescriptorType::InputAttachment);

	/* An input attachment descriptor doesn't have a sampler (because the samples are fragment local). */
	const DescriptorImageInfo info{ nullptr, input.view };
	WriteDescriptorSet write{ hndl, descriptor.layoutBinding.Binding, info };
	write.DescriptorType = DescriptorType::InputAttachment;
	WriteDescriptors({ write });
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const Texture & texture)
{
	ValidateDescriptor(descriptor, DescriptorType::CombinedImageSampler);

	const DescriptorImageInfo info{ texture.Sampler, texture.view };
	const WriteDescriptorSet write{ hndl, descriptor.layoutBinding.Binding, info };
	WriteDescriptors({ write });
}

void Pu::DescriptorSet::Write(const vector<const Descriptor*>& descriptors)
{
	/*
	All descriptors are in the same buffer and have the same binding (i.e. one descriptor block)
		For this we need to create only one WriteDescriptorSet with one DescriptorBufferInfo spanning the block's size.
	All descriptors are in the same buffer, but we have different bindings (i.e. multiple descriptor blocks)
		For this we need to create multiple WriteDescriptorSet's with mutiple DescriptorBufferInfo's with alligned offsets.
		The first rule still applied for descriptors with the same binding.

	We also start the buffer offset from a predefined point, this is because the buffer is shared for all sets from the parent pool.
	*/

	DeviceSize offset = baseOffset;
	std::map<uint32, DescriptorBufferInfo> tmp;
	for (const Descriptor *cur : descriptors)
	{
		/* Make sure that the descriptor is accepted in the operation and get it's size. */
		ValidateDescriptor(*cur, DescriptorType::UniformBuffer);
		const DeviceSize size = cur->GetSize();

		decltype(tmp)::iterator it = tmp.find(cur->GetBinding());
		if (it != tmp.end())
		{
			/* This binding is already found, so we just add the descriptor size to its range. */
			it->second.Range += size;
		}
		else
		{
			/* 
			This binding is not yet found, we need a new DescriptorBufferInfo.
			We also must ensure that the offset within the buffer is handled correctly.
			*/
			offset = cur->GetAllignedOffset(offset);
			tmp.emplace(cur->GetBinding(), DescriptorBufferInfo{ pool->GetBuffer(), offset, size });
		}

		offset += size;
	}

	/* 
	Preallocate the required buffers (required, otherwise the vector pointer will be invalid)
	and fill them with the required data.
	*/
	vector<DescriptorBufferInfo> bufferInfos;
	vector<WriteDescriptorSet> writes;
	bufferInfos.reserve(tmp.size());
	writes.reserve(tmp.size());
	for (const auto&[binding, info] : tmp)
	{
		bufferInfos.emplace_back(info);
		writes.emplace_back(WriteDescriptorSet{ hndl, binding, bufferInfos.back() });
	}

	WriteDescriptors(writes);
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
	if (hndl) pool->Free(hndl);
}