#include "Graphics/Vulkan/Shaders/GraphicsPipeline.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device, size_t maxSets)
	: parent(&device), renderpass(nullptr), hndl(nullptr), maxSets(maxSets),
	PostInitialize("GraphicsPipelinePostInitialze"), pool(nullptr)
{}

Pu::GraphicsPipeline::GraphicsPipeline(LogicalDevice & device, const Renderpass & renderpass, size_t maxSets)
	: parent(&device), renderpass(&renderpass), hndl(nullptr), maxSets(maxSets),
	PostInitialize("GraphicsPipelinePostInitialze"), pool(nullptr)
{
	Initialize();
}

Pu::GraphicsPipeline::GraphicsPipeline(GraphicsPipeline && value)
	: parent(value.parent), renderpass(value.renderpass), pool(value.pool),
	hndl(value.hndl), layoutHndl(value.layoutHndl), vp(value.vp), maxSets(value.maxSets),
	descriptorSets(std::move(value.descriptorSets)), scissor(value.scissor),
	vertexInput(value.vertexInput), inputAssembly(value.inputAssembly),
	tessellation(value.tessellation), display(value.display),
	rasterizer(value.rasterizer), multisample(value.multisample),
	depthStencil(value.depthStencil), colorBlend(value.colorBlend),
	dynamicState(std::move(value.dynamicState)), PostInitialize(std::move(value.PostInitialize)),
	colorBlendAttachments(std::move(value.colorBlendAttachments)),
	bindingDescriptions(std::move(value.bindingDescriptions)), dynamicStates(std::move(dynamicStates))
{
	value.renderpass = nullptr;
	value.hndl = nullptr;
	value.layoutHndl = nullptr;
	value.pool = nullptr;
}

Pu::GraphicsPipeline & Pu::GraphicsPipeline::operator=(GraphicsPipeline && other)
{
	if (this != &other)
	{
		Destroy();

		parent = other.parent;
		PostInitialize = std::move(other.PostInitialize);
		renderpass = other.renderpass;
		pool = other.pool;
		hndl = other.hndl;
		layoutHndl = other.layoutHndl;
		vp = other.vp;
		scissor = other.scissor;
		maxSets = other.maxSets;
		vertexInput = other.vertexInput;
		inputAssembly = other.inputAssembly;
		tessellation = other.tessellation;
		display = other.display;
		rasterizer = other.rasterizer;
		multisample = other.multisample;
		depthStencil = other.depthStencil;
		colorBlend = other.colorBlend;
		dynamicState = other.dynamicState;
		descriptorSets = std::move(other.descriptorSets);
		colorBlendAttachments = std::move(other.colorBlendAttachments);
		bindingDescriptions = std::move(other.bindingDescriptions);
		dynamicStates = std::move(other.dynamicStates);
		
		other.renderpass = nullptr;
		other.hndl = nullptr;
		other.layoutHndl = nullptr;
		other.pool = nullptr;
	}

	return *this;
}

const Pu::Renderpass & Pu::GraphicsPipeline::GetRenderpass(void) const
{
#ifdef _DEBUG
	if (!renderpass) Log::Fatal("Cannot get render pass from graphics pipeline that isn't finalized!");
#endif

	return *renderpass;
}

const Pu::DescriptorPool & Pu::GraphicsPipeline::GetDescriptorPool(void) const
{
#ifdef _DEBUG
	if (!renderpass) Log::Fatal("Cannot get descriptor pool from graphics pipeline that isn't finalized!");
	if (!pool) Log::Fatal("This graphics pipeline doesn't define any descriptors!");
#endif

	return *pool;
}

/* Not all codepaths return a value, Log::Fatal will always throw. */
#pragma warning(push)
#pragma warning(disable:4715)
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
#pragma warning(pop)

void Pu::GraphicsPipeline::AddDynamicState(DynamicState state)
{
	/* No need to throw, just a simple check. */
	if (!dynamicStates.contains(state)) dynamicStates.emplace_back(state);
}

void Pu::GraphicsPipeline::AddVertexBinding(uint32 binding, uint32 stride, VertexInputRate inputRate)
{
	/* Make sure binding isn't added mutliple times. */
	if (bindingDescriptions.contains([binding](const VertexInputBindingDescription &cur) { return cur.Binding == binding; }))
	{
		Log::Error("Attempting to add vertex binding description for already added binding!");
		return;
	}

	/* Add binding. */
	bindingDescriptions.emplace_back(binding, stride, inputRate);
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

	/* Add the vertex descriptions to the final version. */
	const vector<VertexInputAttributeDescription> attrDesc = renderpass->attributes.select<VertexInputAttributeDescription>([](const Attribute &attr) { return attr.description; });
	vertexInput.VertexAttributeDescriptionCount = static_cast<uint32>(renderpass->attributes.size());
	vertexInput.VertexAttributeDescriptions = attrDesc.data();
	vertexInput.VertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size());
	vertexInput.VertexBindingDescriptions = bindingDescriptions.data();

	/* Add the dynamic states to the final version. */
	dynamicState.DynamicStateCount = static_cast<uint32>(dynamicStates.size());
	dynamicState.DynamicStates = dynamicStates.data();

	/* Create the descriptor set layouts and the pipeline layout. */
	FinalizeLayout();

	/* Create the pool from which the user can allocate descriptor sets. */
	if (!renderpass->uniforms.empty()) pool = new DescriptorPool(*this, maxSets);

	/* Create graphics pipeline. */
	const vector<PipelineShaderStageCreateInfo> stages = renderpass->shaders.select<PipelineShaderStageCreateInfo>([](const Shader &shader) { return shader.info; });
	GraphicsPipelineCreateInfo createInfo(stages, layoutHndl, renderpass->hndl);
	createInfo.VertexInputState = &vertexInput;
	createInfo.InputAssemblyState = &inputAssembly;
	createInfo.TessellationState = &tessellation;
	createInfo.ViewportState = &display;
	createInfo.RasterizationState = &rasterizer;
	createInfo.MultisampleState = &multisample;
	createInfo.DepthStencilState = &depthStencil;
	createInfo.ColorBlendState = &colorBlend;
	createInfo.DynamicState = &dynamicState;

	VK_VALIDATE(parent->vkCreateGraphicsPipelines(parent->hndl, nullptr, 1, &createInfo, nullptr, &hndl), PFN_vkCreateGraphicsPipelines);
}

void Pu::GraphicsPipeline::FinalizeLayout(void)
{
	/* Get the information about the descriptor sets. */
	vector<DescriptorSetLayoutCreateInfo> createInfos;
	std::map<uint32, vector<DescriptorSetLayoutBinding>> setBindings;
	for (const Uniform &cur : renderpass->uniforms)
	{
		/* Make sure we have a descriptor set for each defined set. */
		if (cur.set >= createInfos.size())
		{
			/* Add the layout binding for the uniform to the set as well. */
			createInfos.resize(cur.set + 1);
			setBindings.emplace(cur.set, vector<DescriptorSetLayoutBinding>());
			setBindings[cur.set].emplace_back(cur.layoutBinding);
		}
		else
		{
			/* The current set is already defined so search in the list for it. */
			for (DescriptorSetLayoutBinding &binding : setBindings[cur.set])
			{
				if (binding.Binding == cur.layoutBinding.Binding)
				{
					/* Just increase the descriptor count. */
					binding.DescriptorCount += cur.layoutBinding.DescriptorCount;
					goto Found;
				}
			}

			/* The binding wasn't found so just add it like normal. */
			setBindings[cur.set].emplace_back(cur.layoutBinding);

		Found:;
		}
	}

	/* Resize the result vector outside of the loop to increase performance. */
	descriptorSets.resize(createInfos.size(), nullptr);

	/* Create all required descriptor sets. */
	for (size_t i = 0; i < createInfos.size(); i++)
	{
		DescriptorSetLayoutCreateInfo &info = createInfos[i];
		const vector<DescriptorSetLayoutBinding> bindings = setBindings.at(static_cast<uint32>(i));

		/* Set final bindings, this has to be done here as vector resizing might reallocate and thusly will give wrong addresses. */
		info.BindingCount = static_cast<uint32>(bindings.size());
		info.Bindings = bindings.data();

		/* Actually create the set. */
		VK_VALIDATE(parent->vkCreateDescriptorSetLayout(parent->hndl, &info, nullptr, &descriptorSets[i]), PFN_vkCreateDescriptorSetLayout);
	}

	/* Create pipeline layout. */
	const PipelineLayoutCreateInfo layoutCreateInfo(descriptorSets);
	VK_VALIDATE(parent->vkCreatePipelineLayout(parent->hndl, &layoutCreateInfo, nullptr, &layoutHndl), PFN_vkCreatePipelineLayout);
}

void Pu::GraphicsPipeline::Initialize(void)
{
	/* Add blend attachment for all color outputs. */
	for (const Output &cur : renderpass->outputs)
	{
		if (cur.type == OutputUsage::Color) colorBlendAttachments.emplace_back(cur.attachment);
	}

	/* Only allocate the required structures. */
	vertexInput = PipelineVertexInputStateCreateInfo();
	inputAssembly = PipelineInputAssemblyStateCreateInfo(PrimitiveTopology::TriangleList);
	tessellation = PipelineTessellationStateCreateInfo();
	display = PipelineViewportStateCreateInfo(vp, scissor);
	rasterizer = PipelineRasterizationStateCreateInfo(CullModeFlag::Back);
	multisample = PipelineMultisampleStateCreateInfo(SampleCountFlag::Pixel1Bit);
	depthStencil = PipelineDepthStencilStateCreateInfo();
	colorBlend = PipelineColorBlendStateCreateInfo(colorBlendAttachments);
	dynamicState = PipelineDynamicStateCreateInfo();

	/* Allow user to set paramaters. */
	PostInitialize.Post(*this);
}

void Pu::GraphicsPipeline::Destroy(void)
{
	if (hndl)
	{
		parent->vkDestroyPipeline(parent->hndl, hndl, nullptr);
		parent->vkDestroyPipelineLayout(parent->hndl, layoutHndl, nullptr);

		for (DescriptorSetHndl set : descriptorSets) parent->vkDestroyDescriptorSetLayout(parent->hndl, set, nullptr);
		descriptorSets.clear();

		delete pool;
	}
}

Pu::GraphicsPipeline::LoadTask::LoadTask(GraphicsPipeline & pipelineResult, Renderpass & passResult, const vector<std::tuple<size_t, wstring>>& toLoad)
	: result(pipelineResult), renderPass(passResult)
{
	child = new Renderpass::LoadTask(passResult, toLoad);
	child->SetParent(*this);
}

Pu::Task::Result Pu::GraphicsPipeline::LoadTask::Execute(void)
{
	/* Spawn the render pass load task. */
	scheduler->Spawn(*child);
	return Result::Default();
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
	return Result::AutoDelete();
}