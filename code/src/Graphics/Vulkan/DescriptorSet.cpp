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

		parent = other.parent;
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
	DescriptorImageInfo info(texture.Sampler->hndl, texture.view->hndl);
	WriteDescriptorSet write(hndl, uniform.layoutBinding.Binding, info);
	WriteDescriptor({ write });
}

void Pu::DescriptorSet::Write(const vector<const Uniform*>& uniforms, const Buffer & buffer)
{
	/*
	All uniforms are in the same buffer and have the same binding (i.e. one UniformBlock)
		For this we need to create only one WriteDescriptorSet with one DescriptorBufferInfo spanning the entire buffer.
	All uniforms are in the same buffer but have different bindings (i.e. multiple UniformBlocks)
		For this we need to create multiple WriteDescriptorSet's with multiple DescriptorBufferInfo's with alligned offsets.
		The first rule still aplies for descriptors in the same binding.
	*/

	/* Create all the buffers needed for this command. */
	std::map<uint32, DescriptorBufferInfo> temp;
	vector<DescriptorBufferInfo> bufferInfos;
	vector<WriteDescriptorSet> writes;

	DeviceSize offset = 0;
	for (const Uniform *cur : uniforms)
	{
		/* Make sure the passed uniform is usable. */
		if (cur->layoutBinding.DescriptorType != DescriptorType::UniformBuffer) Log::Fatal("Cannot update descriptor with buffer on non-buffer uniform!");

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
	parent->parent->parent->vkUpdateDescriptorSets(parent->parent->parent->hndl, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}

void Pu::DescriptorSet::Free(void)
{
	if (hndl) parent->FreeSet(hndl);
}