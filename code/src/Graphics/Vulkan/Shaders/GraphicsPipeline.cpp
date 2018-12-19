#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

Pu::PipelineColorBlendAttachmentState attach; // TODO: make this proper.

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device, const Renderpass & renderpass)
	: parent(device), renderpass(renderpass), tessellation(nullptr), multisample(nullptr),
	depthStencil(nullptr), colorBlend(nullptr), dynamicState(nullptr), hndl(nullptr)
{
	/* Only allocate the required structures. */
	vertexInput = new PipelineVertexInputStateCreateInfo();
	inputAssembly = new PipelineInputAssemblyStateCreateInfo(PrimitiveTopology::TriangleList);
	display = new PipelineViewportStateCreateInfo(viewport, scissor);
	rasterizer = new PipelineRasterizationStateCreateInfo(CullModeFlag::Back);
	multisample = new PipelineMultisampleStateCreateInfo(SampleCountFlag::Pixel1Bit);
	colorBlend = new PipelineColorBlendStateCreateInfo(attach);
}

void Pu::GraphicsPipeline::Finalize(void)
{
	/* Make sure we don't finalize multiple times! */
	if (hndl)
	{
		Log::Error("Cannot finalize a graphics pipeline multiple times!");
		return;
	}

	/* Create shader stage buffer. */
	vector<PipelineShaderStageCreateInfo> stages;
	for (const Subpass &subpass : renderpass.subpasses)
	{
		stages.push_back(subpass.info);
	}

	/* Create pipeline layout. */
	PipelineLayoutCreateInfo layoutCreateInfo;
	VK_VALIDATE(parent.vkCreatePipelineLayout(parent.hndl, &layoutCreateInfo, nullptr, &layoutHndl), PFN_vkCreatePipelineLayout);

	/* Create graphics pipeline. */
	GraphicsPipelineCreateInfo createInfo(stages, *vertexInput, *inputAssembly, *display, *rasterizer, *multisample, *colorBlend, layoutHndl, renderpass.hndl);
	VK_VALIDATE(parent.vkCreateGraphicsPipelines(parent.hndl, nullptr, 1, &createInfo, nullptr, &hndl), PFN_vkCreateGraphicsPipelines);
}

void Pu::GraphicsPipeline::Destroy(void)
{
	if (hndl)
	{
		parent.vkDestroyPipeline(parent.hndl, hndl, nullptr);
		parent.vkDestroyPipelineLayout(parent.hndl, layoutHndl, nullptr);

		delete vertexInput;
		delete inputAssembly;
		delete display;
		delete rasterizer;
		delete multisample;
		delete colorBlend;
	}
}