#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device)
	: parent(device), renderpass(nullptr), tessellation(nullptr),
	depthStencil(nullptr), dynamicState(nullptr), hndl(nullptr),
	PostInitialize("GraphicsPipelinePostInitialze")
{}

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device, const Renderpass & renderpass)
	: parent(device), renderpass(&renderpass), tessellation(nullptr),
	depthStencil(nullptr), dynamicState(nullptr), hndl(nullptr),
	PostInitialize("GraphicsPipelinePostInitialze")
{
	Initialize();
}

Pu::GraphicsPipeline::~GraphicsPipeline(void)
{
	Destroy();

	if (renderpass)
	{
		delete vertexInput;
		delete inputAssembly;
		delete display;
		delete rasterizer;
		delete multisample;
		delete colorBlend;
	}
}

Pu::PipelineColorBlendAttachmentState & Pu::GraphicsPipeline::GetBlendStateFor(const string & name)
{
	if (!renderpass) Log::Fatal("Unable to request blend state on non-initialized graphics pipeline!");

	/* We have to inline search for the output because the index of the attachment is used to link it to the blend state. */
	for (size_t i = 0; i < renderpass->outputs.size(); i++)
	{
		const Output &cur = renderpass->outputs[i];
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
	if (!renderpass) Log::Fatal("Unable to finalize non-initialized graphics pipeline!");

	/* If finalize is called a second time; first destroy the old one. */
	if (hndl)
	{
		Log::Warning("Recreating graphics pipeline, may cause lag!");
		Destroy();
	}

	/* Create shader stage buffer. */
	vector<PipelineShaderStageCreateInfo> stages;
	for (const Subpass &subpass : renderpass->subpasses)
	{
		stages.push_back(subpass.info);
	}

	/* Create pipeline layout. */
	PipelineLayoutCreateInfo layoutCreateInfo;
	VK_VALIDATE(parent.vkCreatePipelineLayout(parent.hndl, &layoutCreateInfo, nullptr, &layoutHndl), PFN_vkCreatePipelineLayout);

	/* Create graphics pipeline. */
	GraphicsPipelineCreateInfo createInfo(stages, *vertexInput, *inputAssembly, *display, *rasterizer, *multisample, *colorBlend, layoutHndl, renderpass->hndl);
	VK_VALIDATE(parent.vkCreateGraphicsPipelines(parent.hndl, nullptr, 1, &createInfo, nullptr, &hndl), PFN_vkCreateGraphicsPipelines);
}

void Pu::GraphicsPipeline::Initialize(void)
{
	/* Add blend attachment for all color outputs. */
	for (const Output &cur : renderpass->outputs)
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

	/* Allow user to set paramaters. */
	PostInitialize.Post(*this, EventArgs());

	/* We're done loading the graphics pipeline. */
	loaded.store(true);
}

void Pu::GraphicsPipeline::Destroy(void)
{
	if (hndl)
	{
		parent.vkDestroyPipeline(parent.hndl, hndl, nullptr);
		parent.vkDestroyPipelineLayout(parent.hndl, layoutHndl, nullptr);
	}
}

Pu::GraphicsPipeline::LoadTask::LoadTask(GraphicsPipeline & pipelineResult, Renderpass & passResult, std::initializer_list<const char*> subpasses)
	: result(pipelineResult), renderPass(passResult)
{
	child = new Renderpass::LoadTask(passResult, subpasses);
	child->SetParent(*this);
}

Pu::Task::Result Pu::GraphicsPipeline::LoadTask::Execute(void)
{
	/* Spawn the render pass load task. */
	scheduler->Spawn(*child);
	return Result();
}

Pu::Task::Result Pu::GraphicsPipeline::LoadTask::Continue(void)
{
	/* Make sure that the render pass is loaded on debug mode. */
#ifdef _DEBUG
	if (!renderPass.IsLoaded()) Log::Fatal("Render pass failed to load!");
#endif

	/* Destroy child task and set the render pass for the result. */
	delete child;
	result.renderpass = &renderPass;

	/* Initializes the graphics pipeline and return that we're done loading. */
	result.Initialize();
	return Result();
}