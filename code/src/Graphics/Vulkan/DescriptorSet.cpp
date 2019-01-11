#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

Pu::DescriptorSet::DescriptorSet(DescriptorSet && value)
	: parent(value.parent), hndl(value.hndl)
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

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::DescriptorSet::Write(const Uniform & uniform, const Texture & texture)
{
	DescriptorImageInfo info(texture.sampler.hndl, texture.view->hndl);
	WriteImage(uniform, { info });
}

void Pu::DescriptorSet::Write(const Uniform & uniform, const vector<const Texture*>& textures)
{
	vector<DescriptorImageInfo> infos;
	for (const Texture *cur : textures) infos.emplace_back(cur->sampler.hndl, cur->view->hndl);
	WriteImage(uniform, infos);
}

Pu::DescriptorSet::DescriptorSet(DescriptorPool & pool, DescriptorSetHndl hndl)
	: parent(pool), hndl(hndl)
{}

void Pu::DescriptorSet::WriteImage(const Uniform & uniform, const vector<DescriptorImageInfo>& infos)
{
	WriteDescriptorSet write(hndl, uniform.layoutBinding.Binding, infos);
	WriteDescriptor({ write });
}

void Pu::DescriptorSet::WriteDescriptor(const vector<WriteDescriptorSet> & writes)
{
	UpdateDescriptor(writes.size(), writes.data(), 0, nullptr);
}

void Pu::DescriptorSet::UpdateDescriptor(size_t writeCount, const WriteDescriptorSet * writes, size_t copyCount, const CopyDescriptorSet * copies)
{
	parent.parent.parent.vkUpdateDescriptorSets(parent.parent.parent.hndl, static_cast<uint32>(writeCount), writes, static_cast<uint32>(copyCount), copies);
}

void Pu::DescriptorSet::Free(void)
{
	if (hndl) parent.FreeSet(hndl);
}