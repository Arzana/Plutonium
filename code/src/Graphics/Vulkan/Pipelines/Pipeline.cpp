#include "Graphics/Vulkan/Pipelines/Pipeline.h"

Pu::Pipeline::Pipeline(Pipeline && value)
	: Hndl(value.Hndl), layout(std::move(value.layout)),
	shaderStages(std::move(value.shaderStages))
{}

Pu::Pipeline & Pu::Pipeline::operator=(Pipeline && other)
{
	if (this != &other)
	{
		Destroy();
		Hndl = other.Hndl;
		layout = std::move(other.layout);
		shaderStages = std::move(other.shaderStages);

		other.Hndl = nullptr;
	}

	return *this;
}

Pu::Pipeline::Pipeline(LogicalDevice & device, const Subpass & subpass)
	: Hndl(nullptr), layout(device, subpass)
{
	/* Get all the information from the shaders. */
	shaderStages.reserve(subpass.GetShaders().size());
	for (const Shader *shader : subpass.GetShaders())
	{
		shaderStages.emplace_back(shader->info);
	}
}

void Pu::Pipeline::Destroy(void)
{
	if (Hndl) layout.device->vkDestroyPipeline(layout.device->hndl, Hndl, nullptr);
}