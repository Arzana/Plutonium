#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: parent(value.parent), device(value.device), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::DescriptorPool & Pu::DescriptorPool::operator=(DescriptorPool && other)
{
	if (this != &other)
	{
		Destroy();
		parent = other.parent;
		device = other.device;
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::DescriptorSet Pu::DescriptorPool::Allocate(uint32 set) const
{
	/* Initialize creation info. */
	const DescriptorSetAllocateInfo allocInfo(hndl, parent->descriptorSets.at(set));
	DescriptorSetHndl setHndl;

	/* Allocate new descriptor set. */
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &allocInfo, &setHndl), PFN_vkAllocateDescriptorSets);
	return DescriptorSet(const_cast<DescriptorPool&>(*this), setHndl, set);
}

Pu::DescriptorPool::DescriptorPool(GraphicsPipeline & parent, size_t maxSets)
	: parent(&parent), device(parent.parent)
{
	/* Determine how many of each descriptor type are needed. */
	vector<DescriptorPoolSize> sizes;
	for (const Uniform &uniform : parent.renderpass->uniforms)
	{
		/* Check if size already exists. */
		for (DescriptorPoolSize &size : sizes)
		{
			if (size.Type == uniform.GetDescriptorType())
			{
				/* Increase the descriptor count and skip creating a new pool size by goto. */
				++size.DescriptorCount;
				goto Continue;
			}
		}

		/* If the type was not found then just add a new one with one count. */
		sizes.emplace_back(uniform.GetDescriptorType(), 1);

	Continue:;
	}

	/* Create the pool. */
	const DescriptorPoolCreateInfo createInfo(static_cast<uint32>(maxSets), sizes);
	VK_VALIDATE(device->vkCreateDescriptorPool(device->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateDescriptorPool);
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl) device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
}

void Pu::DescriptorPool::FreeSet(DescriptorSetHndl set) const
{
	device->vkFreeDescriptorSets(device->hndl, hndl, 1, &set);
}