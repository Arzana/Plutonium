#include "Graphics/Vulkan/Pipelines/PipelineLayout.h"

Pu::PipelineLayout::PipelineLayout(LogicalDevice & device, const Subpass & subpass)
	: device(&device)
{
	CreateDescriptorSetLayouts(subpass);
	CreatePipelineLayout(subpass);
}

Pu::PipelineLayout::PipelineLayout(PipelineLayout && value)
	: device(value.device), hndl(value.hndl), setHndls(std::move(value.setHndls))
{
	value.hndl = nullptr;
}

Pu::PipelineLayout & Pu::PipelineLayout::operator=(PipelineLayout && other)
{
	if (this != &other)
	{
		Destroy();

		device = other.device;
		hndl = other.hndl;
		setHndls = std::move(other.setHndls);

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::PipelineLayout::Destroy(void)
{
	if (hndl) device->vkDestroyPipelineLayout(device->hndl, hndl, nullptr);

	for (DescriptorSetLayoutHndl cur : setHndls)
	{
		device->vkDestroyDescriptorSetLayout(device->hndl, cur, nullptr);
	}
}

void Pu::PipelineLayout::CreateDescriptorSetLayouts(const Subpass & subpass)
{
	/*
	We need to create a set layout for each descriptor set defined in the subpass. 
	Every one if these sets will probably have multiple bindings associated with it.
	*/
	std::map<uint32, vector<DescriptorSetLayoutBinding>> layoutBindings;
	for (const Descriptor &descriptor : subpass.descriptors) 
	{
		/* Check if we already added the set to the map. */
		decltype(layoutBindings)::iterator it = layoutBindings.find(descriptor.set);
		if (it != layoutBindings.end())
		{
			/* Only add a new layout binding to the set if it's not yet in use. */
			if (!it->second.contains([&descriptor](const DescriptorSetLayoutBinding &cur) { return cur.Binding == descriptor.GetBinding(); }))
			{
				it->second.emplace_back(descriptor.layoutBinding);
			}
		}
		else
		{
			/* Just add the descriptor to the list with its set. */
			vector<DescriptorSetLayoutBinding> value = { descriptor.layoutBinding };
			layoutBindings.emplace(descriptor.set, std::move(value));
		}
	}

	/* Presize the output array so we don't resize on every create call. */
	setHndls.resize(layoutBindings.size());
	DescriptorSetLayoutHndl *output = setHndls.data();

	/* Create the descriptor set layouts. */
	for (const auto &[set, bindings] : layoutBindings)
	{
		const DescriptorSetLayoutCreateInfo createInfo{ bindings };
		VK_VALIDATE(device->vkCreateDescriptorSetLayout(device->hndl, &createInfo, nullptr, output++), PFN_vkCreateDescriptorSetLayout);
	}
}

void Pu::PipelineLayout::CreatePipelineLayout(const Subpass & subpass)
{
	/* Scan the subpass for push constant ranges. */
	vector<PushConstantRange> pushRanges;
	for (const PushConstant &pushConstant : subpass.pushConstants)
	{
		/* Add the size of the push constant if we find multiple in the same shader stage. */
		bool add = true;
		for (PushConstantRange &range : pushRanges)
		{
			if (range.StageFlags == pushConstant.range.StageFlags)
			{
				range.Size += static_cast<uint32>(pushConstant.GetSize());
				add = false;
				break;
			}
		}

		/* Otherwise add a new range to the list. */
		if (add) pushRanges.emplace_back(pushConstant.range);
	}

	/* Create the pipeline layout. */
	const PipelineLayoutCreateInfo createInfo{ setHndls, pushRanges };
	VK_VALIDATE(device->vkCreatePipelineLayout(device->hndl, &createInfo, nullptr, &hndl), PFN_vkCreatePipelineLayout);
}