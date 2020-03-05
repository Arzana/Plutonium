#include "Graphics/Vulkan/Pipelines/Pipeline.h"

Pu::Pipeline::Pipeline(Pipeline && value)
	: Hndl(value.Hndl), LayoutHndl(value.LayoutHndl), Device(value.Device),
	shaderStages(std::move(value.shaderStages))
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

	/* Create the pipeline layout. */
	CreatePipelineLayout(subpass);
}

void Pu::Pipeline::Destroy(void)
{
	if (Hndl) Device->vkDestroyPipeline(Device->hndl, Hndl, nullptr);
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

	/* Just get a list of the descriptor set Vulkan handles. */
	vector<DescriptorSetHndl> hndls;
	hndls.reserve(subpass.setLayouts.size());
	for (const DescriptorSetLayout &layout : subpass.setLayouts) hndls.emplace_back(layout.hndl);

	/* Create the pipeline layout. */
	const PipelineLayoutCreateInfo createInfo{ hndls, pushRanges };
	VK_VALIDATE(Device->vkCreatePipelineLayout(Device->hndl, &createInfo, nullptr, &LayoutHndl), PFN_vkCreatePipelineLayout);
}

void Pu::Pipeline::FullDestroy(void)
{
	/*
	First destroy the actual pipeline.
	After that; destroy the layouts.
	*/
	Destroy();
	if (LayoutHndl) Device->vkDestroyPipelineLayout(Device->hndl, LayoutHndl, nullptr);
}