#include "Graphics/Vulkan/Pipelines/Pipeline.h"

Pu::Pipeline::Pipeline(Pipeline && value)
	: Hndl(value.Hndl), LayoutHndl(value.LayoutHndl), Device(value.Device),
	shaderStages(std::move(value.shaderStages)), setHndls(std::move(value.setHndls))
{
	value.Hndl = nullptr;
	value.LayoutHndl = nullptr;
}

Pu::Pipeline & Pu::Pipeline::operator=(Pipeline && other)
{
	if (this != &other)
	{
		FullDestroy();
		
		Hndl = other.Hndl;
		LayoutHndl = other.LayoutHndl;
		Device = other.Device;
		shaderStages = std::move(other.shaderStages);
		setHndls = std::move(other.setHndls);

		other.Hndl = nullptr;
		other.LayoutHndl = nullptr;
	}

	return *this;
}

Pu::Pipeline::Pipeline(LogicalDevice & device, const Subpass & subpass)
	: Device(&device)
{
	/* Set the shader stage information. */
	shaderStages.reserve(subpass.GetShaders().size());
	for (const Shader *shader : subpass.GetShaders())
	{
		shaderStages.emplace_back(shader->info);
	}

	/* Create the descriptor set layouts and the pipeline layout. */
	CreateDescriptorSetLayouts(subpass);
	CreatePipelineLayout(subpass);
}

void Pu::Pipeline::Destroy(void)
{
	if (Hndl) Device->vkDestroyPipeline(Device->hndl, Hndl, nullptr);
}

void Pu::Pipeline::CreateDescriptorSetLayouts(const Subpass & subpass)
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
		VK_VALIDATE(Device->vkCreateDescriptorSetLayout(Device->hndl, &createInfo, nullptr, output++), PFN_vkCreateDescriptorSetLayout);
	}
}

void Pu::Pipeline::CreatePipelineLayout(const Subpass & subpass)
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
	VK_VALIDATE(Device->vkCreatePipelineLayout(Device->hndl, &createInfo, nullptr, &LayoutHndl), PFN_vkCreatePipelineLayout);
}

void Pu::Pipeline::FullDestroy(void)
{
	/* 
	First destroy the actual pipeline.
	After that; destroy all of the layouts.
	*/
	Destroy();

	if (LayoutHndl) Device->vkDestroyPipelineLayout(Device->hndl, LayoutHndl, nullptr);
	for (DescriptorSetLayoutHndl cur : setHndls)
	{
		Device->vkDestroyDescriptorSetLayout(Device->hndl, cur, nullptr);
	}
}