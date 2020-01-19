#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"
#include "Graphics/Vulkan/DescriptorPool.h"

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

		parent = other.parent;
		hndl = other.hndl;
		set = other.set;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Descriptor & descriptor, const ImageView & view, const Sampler & sampler)
{
	/* Check if the descriptor type is correct. */
	if (descriptor.layoutBinding.DescriptorType != DescriptorType::CombinedImageSampler) Log::Fatal("Cannot update descriptor with image on non-image descriptor!");

	/* Write the descriptor. */
	DescriptorImageInfo info(sampler.hndl, view.hndl);
	WriteDescriptorSet write(hndl, descriptor.layoutBinding.Binding, info);
	WriteDescriptor({ write });
}

void Pu::DescriptorSet::Write(const vector<const Descriptor*>& descriptors, const Buffer & buffer)
{
	/*
	All descriptors are in the same buffer and have the same binding (i.e. one descriptorBlock)
		For this we need to create only one WriteDescriptorSet with one DescriptorBufferInfo spanning the entire buffer.
	All descriptors are in the same buffer but have different bindings (i.e. multiple descriptorBlocks)
		For this we need to create multiple WriteDescriptorSet's with multiple DescriptorBufferInfo's with alligned offsets.
		The first rule still aplies for descriptors in the same binding.
	*/

	/* Create all the buffers needed for this command. */
	std::map<uint32, DescriptorBufferInfo> temp;
	vector<DescriptorBufferInfo> bufferInfos;
	vector<WriteDescriptorSet> writes;

	DeviceSize offset = 0;
	for (const Descriptor *cur : descriptors)
	{
		/* Make sure the passed descriptor is usable. */
		if (cur->layoutBinding.DescriptorType != DescriptorType::UniformBuffer) Log::Fatal("Cannot update descriptor with buffer on non-buffer descriptor!");

		/* Either add a new buffer info if it already exists or resize it if we already have a copy. */
		decltype(temp)::iterator it = temp.find(cur->GetBinding());
		if (it != temp.end())
		{
			it->second.Range += cur->GetSize();
			offset += cur->GetSize();
		}
		else
		{
			offset = cur->GetAllignedOffset(offset);
			temp.emplace(cur->GetBinding(), DescriptorBufferInfo(buffer.bufferHndl, offset, cur->GetSize()));
			offset += cur->GetSize();
		}
	}

	/* Reserve to avoid memory locations changing as we add the values. */
	bufferInfos.reserve(temp.size());
	writes.reserve(temp.size());

	/* Create the final command parameters. */
	for (const auto[binding, info] : temp)
	{
		bufferInfos.emplace_back(info);
		writes.emplace_back(WriteDescriptorSet(hndl, binding, bufferInfos.back()));
	}

	/* Update the descriptors. */
	WriteDescriptor(writes);
}

Pu::DescriptorSet::DescriptorSet(DescriptorPool & pool, DescriptorSetHndl hndl, uint32 set)
	: parent(&pool), hndl(hndl), set(set)
{}

void Pu::DescriptorSet::WriteDescriptor(const vector<WriteDescriptorSet> & writes)
{
	parent->device->vkUpdateDescriptorSets(parent->device->hndl, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}

void Pu::DescriptorSet::Free(void)
{
	if (hndl) parent->FreeSet(hndl);
}