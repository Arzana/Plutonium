#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Vulkan/Shaders/Renderpass.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, size_t maxSets)
	: renderpass(&renderpass), max(static_cast<uint32>(maxSets)), used(0)
{
	/* Determine how many of each descriptor type are needed. */
	vector<DescriptorPoolSize> sizes;
	for (const Subpass &subpass : renderpass.subpasses)
	{
		for (const Descriptor &descriptor : subpass.descriptors)
		{
			/*
			Check if the descriptor type is already defined,
			if it is, then just increate the size by one,
			otherwise; add the type with a size of one.
			*/
			decltype(sizes)::iterator it = sizes.iteratorOf([&descriptor](const DescriptorPoolSize &cur) { return cur.Type == descriptor.GetType(); });
			if (it != sizes.end()) ++it->DescriptorCount;
			else sizes.emplace_back(descriptor.GetType(), 1);
		}
	}

	/* Create the pool. */
	const DescriptorPoolCreateInfo createInfo(static_cast<uint32>(maxSets), sizes);
	VK_VALIDATE(renderpass.device->vkCreateDescriptorPool(renderpass.device->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateDescriptorPool);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: renderpass(value.renderpass), hndl(value.hndl), max(value.max), used(value.used)
{
	value.hndl = nullptr;
}

Pu::DescriptorPool & Pu::DescriptorPool::operator=(DescriptorPool && other)
{
	if (this != &other)
	{
		Destroy();

		renderpass = other.renderpass;
		hndl = other.hndl;
		max = other.max;
		used = other.used;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::DescriptorSet Pu::DescriptorPool::Allocate(uint32 set) const
{
	/* Make sure we don't alocate over our maximum. */
	if (++used > max) Log::Error("Cannot allocate more the maximum defined amount of sets!");

	/* Initialize creation info. */
	const DescriptorSetAllocateInfo allocInfo(hndl, renderpass->descriptorSetLayouts.at(set));
	DescriptorSetHndl setHndl;

	/* Allocate new descriptor set. */
	VK_VALIDATE(renderpass->device->vkAllocateDescriptorSets(renderpass->device->hndl, &allocInfo, &setHndl), PFN_vkAllocateDescriptorSets);
	return DescriptorSet(const_cast<DescriptorPool&>(*this), setHndl, set);
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl) renderpass->device->vkDestroyDescriptorPool(renderpass->device->hndl, hndl, nullptr);
}

void Pu::DescriptorPool::FreeSet(DescriptorSetHndl set) const
{
	--used;
	renderpass->device->vkFreeDescriptorSets(renderpass->device->hndl, hndl, 1, &set);
}