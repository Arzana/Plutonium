#include "Graphics/Vulkan/Pipelines/ComputePipeline.h"

Pu::ComputePipeline::ComputePipeline(const ShaderProgram & program)
	: Pipeline(program.GetShaders().front()->GetDevice(), program), program(&program)
{}

void Pu::ComputePipeline::Finalize(void)
{
	/* Destroy the old pipeline if this is a recreate call. */
	if (Hndl)
	{
		Device->WaitIdle();
		Destroy();
	}

	/* Create the new compute pipeline. */
	ComputePipelineCreateInfo createInfo{ GetShaderStages().front(), CreateFlags, LayoutHndl };
	VK_VALIDATE(Device->vkCreateComputePipelines(Device->hndl, nullptr, 1, &createInfo, nullptr, &Hndl), PFN_vkCreateComputePipelines);
}