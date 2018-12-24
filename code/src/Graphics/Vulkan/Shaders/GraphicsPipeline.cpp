#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device, const Renderpass & renderpass)
	: parent(device), renderpass(renderpass), tessellation(nullptr),
	depthStencil(nullptr), dynamicState(nullptr), hndl(nullptr)
{
	/* Add blend attachment for all color outputs. */
	for (const Output &cur : renderpass.outputs)
	{
		if (cur.type == OutputUsage::Color) colorBlendAttachments.emplace_back();
	}

	/* Only allocate the required structures. */
	vertexInput = new PipelineVertexInputStateCreateInfo();
	inputAssembly = new PipelineInputAssemblyStateCreateInfo(PrimitiveTopology::TriangleList);
	display = new PipelineViewportStateCreateInfo(viewport, scissor);
	rasterizer = new PipelineRasterizationStateCreateInfo(CullModeFlag::Back);
	multisample = new PipelineMultisampleStateCreateInfo(SampleCountFlag::Pixel1Bit);
	colorBlend = new PipelineColorBlendStateCreateInfo(colorBlendAttachments);
}

Pu::GraphicsPipeline::~GraphicsPipeline(void)
{
	Destroy();

	delete vertexInput;
	delete inputAssembly;
	delete display;
	delete rasterizer;
	delete multisample;
	delete colorBlend;
}

Pu::PipelineColorBlendAttachmentState & Pu::GraphicsPipeline::GetBlendStateFor(const string & name)
{
	/* We have to inline search for the output because the index of the attachment is used to link it to the blend state. */
	for (size_t i = 0; i < renderpass.outputs.size(); i++)
	{
		const Output &cur = renderpass.outputs[i];
		if (cur.Info.Name == name)
		{
			/* Throw if the attachment requested is not a color attachment. */
			if (cur.type == OutputUsage::Color) return colorBlendAttachments[i];
			else Log::Fatal("Attempting to request color blend state of non-color output '%s'!", name.c_str());
		}
	}

	/* Throw if no attachment could be found. */
	Log::Fatal("Cannot get color blend state for unknown output '%s'!", name.c_str());
}

void Pu::GraphicsPipeline::Finalize(void)
{
	/* If finalize is called a second time; first destroy the old one. */
	if (hndl)
	{
		Log::Warning("Recreating graphics pipeline, may cause lag!");
		Destroy();
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
	}
}