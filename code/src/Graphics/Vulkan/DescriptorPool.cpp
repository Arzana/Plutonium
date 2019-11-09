#include "Graphics/Vulkan/DescriptorPool.h"
#include "Graphics/Vulkan/Shaders/Renderpass.h"

Pu::DescriptorPool::DescriptorPool(const Renderpass & renderpass, const Subpass & subpass, uint32 set, size_t maxSets)
	: subpass(&subpass), max(static_cast<uint32>(maxSets)), used(0), set(set),
	device(&subpass.shaders.front()->GetDevice()), pipelineLayout(renderpass.layoutHndl),
	descriptorLayout(renderpass.descriptorSetLayouts[set])
{
	vector<DescriptorPoolSize> sizes;
	for (const Descriptor &descriptor : subpass.descriptors)
	{
		if (descriptor.GetSet() != set) continue;

			/*
			Check if the descriptor type is already defined,
			if it is, then just increate the size by one,
			otherwise; add the type with a size of one.
			*/
			decltype(sizes)::iterator it = sizes.iteratorOf([&descriptor](const DescriptorPoolSize &cur) { return cur.Type == descriptor.GetType(); });
			if (it != sizes.end()) ++it->DescriptorCount;
			else sizes.emplace_back(descriptor.GetType(), 1);
	}

	/* The amount of descriptors are global, so we need to multiply our origional count by the maximum number of sets we want to allocate. */
	for (DescriptorPoolSize &cur : sizes) cur.DescriptorCount *= max;

	/* Create the pool. */
	const DescriptorPoolCreateInfo createInfo(max, sizes);
	VK_VALIDATE(device->vkCreateDescriptorPool(device->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateDescriptorPool);
}

Pu::DescriptorPool::DescriptorPool(DescriptorPool && value)
	: subpass(value.subpass), device(value.device), hndl(value.hndl),
	pipelineLayout(value.pipelineLayout), descriptorLayout(value.descriptorLayout),
	max(value.max), used(value.used), set(value.set)
{
	value.hndl = nullptr;
}

Pu::DescriptorPool & Pu::DescriptorPool::operator=(DescriptorPool && other)
{
	if (this != &other)
	{
		Destroy();

		subpass = other.subpass;
		device = other.device;
		hndl = other.hndl;
		pipelineLayout = other.pipelineLayout;
		descriptorLayout = other.descriptorLayout;
		max = other.max;
		used = other.used;
		set = other.set;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::DescriptorSet Pu::DescriptorPool::Allocate(void) const
{
	/* Make sure we don't alocate over our maximum. */
	if (used >= max) Log::Fatal("Cannot allocate more the maximum defined amount of sets!");
	used++;

	/* Initialize creation info. */
	const DescriptorSetAllocateInfo allocInfo(hndl, descriptorLayout);
	DescriptorSetHndl setHndl;

	/* Allocate new descriptor set. */
	VK_VALIDATE(device->vkAllocateDescriptorSets(device->hndl, &allocInfo, &setHndl), PFN_vkAllocateDescriptorSets);
	return DescriptorSet(const_cast<DescriptorPool&>(*this), setHndl, set);
}

void Pu::DescriptorPool::Destroy(void)
{
	if (hndl) device->vkDestroyDescriptorPool(device->hndl, hndl, nullptr);
}

void Pu::DescriptorPool::FreeSet(DescriptorSetHndl setHndl) const
{
	--used;
	device->vkFreeDescriptorSets(device->hndl, hndl, 1, &setHndl);
}