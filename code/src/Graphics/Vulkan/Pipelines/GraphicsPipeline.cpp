#include "Graphics/Vulkan/Pipelines/GraphicsPipeline.h"

Pu::GraphicsPipeline::GraphicsPipeline(const Renderpass & renderpass, uint32 subpass)
	: Pipeline(*renderpass.device, renderpass.GetSubpass(subpass)), 
	renderpass(&renderpass), subpass(subpass)
{
	/* Initialize the viewport state. */
	viewportState.ViewportCount = 1;
	viewportState.Viewports = &viewport;

	viewportState.ScissorCount = 1;
	viewportState.Scissors = &scissor;

	/* Initialize the color attachments. */
	for (const Output &output : renderpass.subpasses[subpass].outputs)
	{
		if (output.type == OutputUsage::Color) colorBlendAttachments.emplace_back(output.attachment);
	}

	colorBlendState.AttachmentCount = static_cast<uint32>(colorBlendAttachments.size());
	colorBlendState.Attachments = colorBlendAttachments.data();
}

Pu::GraphicsPipeline::GraphicsPipeline(GraphicsPipeline && value)
	: Pipeline(std::move(value)), renderpass(value.renderpass),
	vertexInputState(std::move(value.vertexInputState)), inputAssemblyState(std::move(value.inputAssemblyState)),
	tessellationState(std::move(value.tessellationState)), viewportState(std::move(value.viewportState)),
	rasterizationState(std::move(value.rasterizationState)), depthStencilState(std::move(value.depthStencilState)),
	colorBlendState(std::move(value.colorBlendState)), dynamicState(std::move(value.dynamicState)),
	colorBlendAttachments(std::move(value.colorBlendAttachments)), bindingDescriptions(std::move(value.bindingDescriptions)),
	dynamicStates(std::move(value.dynamicStates)), viewport(value.viewport), scissor(value.scissor), sampleMask(value.sampleMask)
{
	viewportState.Viewports = &viewport;
	viewportState.Scissors = &scissor;
	colorBlendState.AttachmentCount = static_cast<uint32>(colorBlendAttachments.size());
	colorBlendState.Attachments = colorBlendAttachments.data();
	if (multisampleState.SampleMask) multisampleState.SampleMask = &sampleMask;
}

Pu::GraphicsPipeline & Pu::GraphicsPipeline::operator=(GraphicsPipeline && other)
{
	if (this != &other)
	{
		Pipeline::operator=(std::move(other));
		renderpass = other.renderpass;

		vertexInputState = std::move(other.vertexInputState);
		inputAssemblyState = std::move(other.inputAssemblyState);
		tessellationState = std::move(other.tessellationState);
		viewportState = std::move(other.viewportState);
		rasterizationState = std::move(other.rasterizationState);
		multisampleState = std::move(other.multisampleState);
		depthStencilState = std::move(other.depthStencilState);
		colorBlendState = std::move(other.colorBlendState);
		dynamicState = std::move(other.dynamicState);

		colorBlendAttachments = std::move(other.colorBlendAttachments);
		bindingDescriptions = std::move(other.bindingDescriptions);
		dynamicStates = std::move(other.dynamicStates);
		viewport = other.viewport;
		scissor = other.scissor;
		sampleMask = other.sampleMask;

		viewportState.Viewports = &viewport;
		viewportState.Scissors = &scissor;
		colorBlendState.AttachmentCount = static_cast<uint32>(colorBlendAttachments.size());
		colorBlendState.Attachments = colorBlendAttachments.data();
		if (multisampleState.SampleMask) multisampleState.SampleMask = &sampleMask;
	}

	return *this;
}

void Pu::GraphicsPipeline::Finalize(void)
{
	/*
	Just destroy the old one if the graphics pipeline needs to be recreated.
	We do need to wait for the old one to become available again so just wait idle here.
	*/
	if (Hndl)
	{
		renderpass->device->WaitIdle();
		Destroy();
	}

	/* Add the vertex input binding descriptions to the final create information. */
	vector<VertexInputAttributeDescription> attributeDescriptions;
	for (const Attribute &attrib : renderpass->subpasses[subpass].attributes)
	{
		attributeDescriptions.emplace_back(attrib.description);

		/* Check if this vertex attribute is a matrix type, if so add the remaining descriptions. */
		const FieldType &type = attrib.GetInfo().Type;
		if (type.IsMatrix())
		{
			VertexInputAttributeDescription desc = attrib.description;

			/* 
			Calculate the byte size of the matrix,
			from that get the column size (2, 3, or 4),
			and from that get the row size (8, 12, 16).
			*/
			const size_t byteSize = type.GetSize();
			const uint32 columnSize = static_cast<uint32>(sqrtf(byteSize * 0.25f));
			const uint32 rowSize = columnSize << 2;

			/* Add another vertex attribute for every additional dimension. */
			for (uint32 i = 1; i < columnSize; i++)
			{
				desc.Location = attrib.description.Location + i;
				desc.Offset = attrib.description.Location + i * rowSize;
				attributeDescriptions.emplace_back(desc);
			}
		}
	}

	/* Finalize the vertex input state. */
	vertexInputState.VertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
	vertexInputState.VertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputState.VertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size());
	vertexInputState.VertexBindingDescriptions = bindingDescriptions.data();

	/* Finalize the dynamic state. */
	dynamicState.DynamicStateCount = static_cast<uint32>(dynamicStates.size());
	dynamicState.DynamicStates = dynamicStates.data();

	/* Create the graphics pipeline. */
	GraphicsPipelineCreateInfo createInfo{ GetShaderStages(), LayoutHndl, renderpass->hndl, subpass };
	createInfo.VertexInputState = &vertexInputState;
	createInfo.InputAssemblyState = &inputAssemblyState;
	createInfo.TessellationState = &tessellationState;
	createInfo.ViewportState = &viewportState;
	createInfo.RasterizationState = &rasterizationState;
	createInfo.MultisampleState = &multisampleState;
	createInfo.DepthStencilState = &depthStencilState;
	createInfo.ColorBlendState = &colorBlendState;
	createInfo.DynamicState = &dynamicState;
	VK_VALIDATE(renderpass->device->vkCreateGraphicsPipelines(renderpass->device->hndl, nullptr, 1, &createInfo, nullptr, &Hndl), PFN_vkCreateGraphicsPipelines);
}

void Pu::GraphicsPipeline::SetTopology(PrimitiveTopology topology)
{
	inputAssemblyState.Topology = topology;
}

bool Pu::GraphicsPipeline::SetPatchControlPoints(uint32 points)
{
	/* Make sure our hardware supports tessellation shaders. */
	if (GetHardwareEnabled().TessellationShader)
	{
		tessellationState.PatchControlPoints = points;
		return true;
	}

	return false;
}

/* viewport and scissor hide class members. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::GraphicsPipeline::SetViewport(const Viewport & viewport)
{
	this->viewport = viewport;
	this->scissor = viewport.GetScissor();
}

void Pu::GraphicsPipeline::SetViewport(const Viewport & viewport, Rect2D scissor)
{
	this->viewport = viewport;
	this->scissor = scissor;
}
#pragma warning(pop)

bool Pu::GraphicsPipeline::EnableDepthClamp(void)
{
	if (GetHardwareEnabled().DepthClamp)
	{
		rasterizationState.DepthClampEnable = true;
		return true;
	}

	return false;
}

bool Pu::GraphicsPipeline::SetPolygonMode(PolygonMode mode)
{
	/* Fill mode is always supported but line and point might not be. */
	if ((mode != PolygonMode::Fill && GetHardwareEnabled().FillModeNonSolid) || mode == PolygonMode::Fill)
	{
		rasterizationState.PolygonMode = mode;
		return true;
	}

	return false;
}

void Pu::GraphicsPipeline::SetCullMode(CullModeFlag mode)
{
	rasterizationState.CullMode = mode;
}

void Pu::GraphicsPipeline::SetFrontFace(FrontFace front)
{
	rasterizationState.FrontFace = front;
}

bool Pu::GraphicsPipeline::EnableDepthBias(float constant, float clamp, float slope)
{
	rasterizationState.DepthBiasEnable = true;
	rasterizationState.DepthBiasConstantFactor = constant;
	rasterizationState.DepthBiasSlopeFactor = slope;

	/* Only the clamp for the depth bias is hardware dependent. */
	if (GetHardwareEnabled().DepthBiasClamp)
	{
		rasterizationState.DepthBiasClamp = clamp;
		return true;
	}

	return false;
}

bool Pu::GraphicsPipeline::SetLineWidth(float width)
{
	if (GetHardwareEnabled().WideLines)
	{
		rasterizationState.LineWidth = width;
		return true;
	}

	return false;
}

void Pu::GraphicsPipeline::SetSampleCount(SampleCountFlag samples)
{
	multisampleState.RasterizationSamples = samples;
}

void Pu::GraphicsPipeline::EnableSampleShading(float min)
{
	multisampleState.SampleShading = true;
	multisampleState.MinSampleShading = min;
}

void Pu::GraphicsPipeline::SetSampleMask(SampleMask mask)
{
	sampleMask = mask;
	multisampleState.SampleMask = &sampleMask;
}

void Pu::GraphicsPipeline::EnableAlphaToCoverage(void)
{
	multisampleState.AlphaToCoverageEnable = true;
}

bool Pu::GraphicsPipeline::EnableAlphaToOne(void)
{
	if (GetHardwareEnabled().AlphaToOne)
	{
		multisampleState.AlphaToOneEnable = true;
		return true;
	}

	return false;
}

void Pu::GraphicsPipeline::EnableDepthTest(bool enableWrites, CompareOp operation)
{
	depthStencilState.DepthTestEnable = true;
	depthStencilState.DepthWriteEnable = enableWrites;
	depthStencilState.DepthCompareOp = operation;
}

bool Pu::GraphicsPipeline::EnableDepthBoundsTest(float min, float max)
{
	if (GetHardwareEnabled().DepthBounds)
	{
		depthStencilState.DepthBoundsTestEnable = true;
		depthStencilState.MinDepthBounds = min;
		depthStencilState.MaxDepthBounds = max;
		return true;
	}

	return false;
}

void Pu::GraphicsPipeline::EnableStencilTest(const StencilOpState & front, const StencilOpState & back)
{
	depthStencilState.StencilTestEnable = true;
	depthStencilState.Front = front;
	depthStencilState.Back = back;
}

bool Pu::GraphicsPipeline::SetBlendOperation(LogicOp operation)
{
	if (GetHardwareEnabled().LogicOp)
	{
		colorBlendState.LogicOpEnable = true;
		colorBlendState.LogicOp = operation;
		return true;
	}

	return false;
}

void Pu::GraphicsPipeline::SetConstantBlendColor(Color color)
{
	const Vector4 value = color.ToVector4();
	colorBlendState.BlendConstants[0] = value.X;
	colorBlendState.BlendConstants[1] = value.Y;
	colorBlendState.BlendConstants[2] = value.Z;
	colorBlendState.BlendConstants[3] = value.W;
}

void Pu::GraphicsPipeline::AddDynamicState(DynamicState state)
{
	/* Make sure that we don't add the same state twice. */
	if (!dynamicStates.contains(state)) dynamicStates.emplace_back(state);
}

void Pu::GraphicsPipeline::AddVertexBinding(uint32 binding, uint32 stride, VertexInputRate rate)
{
	/* Update the stride and the input rate if it's already defined, otherwise just add it. */
	decltype(bindingDescriptions)::iterator it = bindingDescriptions.iteratorOf([binding](const VertexInputBindingDescription &cur) { return cur.Binding == binding; });
	if (it != bindingDescriptions.end())
	{
		it->Stride = stride;
		it->InputRate = rate;
	}
	else bindingDescriptions.emplace_back(binding, stride, rate);
}

Pu::PipelineColorBlendAttachmentState & Pu::GraphicsPipeline::GetBlendState(const string & name)
{
	static PipelineColorBlendAttachmentState def;

	uint32 index = 0;
	for (const Output &output : renderpass->subpasses[subpass].outputs)
	{
		if (output.GetInfo().Name == name)
		{
			if (output.type == OutputUsage::Color) return colorBlendAttachments[index];
			else Log::Fatal("Attempting to request color blend state of non-color output!");
		}

		++index;
	}

	Log::Error("Could not get blend state for unknown output '%s'!", name.c_str());
	return def;
}

Pu::uint32 Pu::GraphicsPipeline::GetVertexStride(uint32 binding) const
{
	for (const VertexInputBindingDescription &cur : bindingDescriptions)
	{
		if (cur.Binding == binding) return cur.Stride;
	}

	Log::Error("Could not get vertex stride for graphics pipeline (binding %u not found)!", binding);
	return 0;
}

const Pu::PhysicalDeviceFeatures & Pu::GraphicsPipeline::GetHardwareEnabled(void) const
{
	return renderpass->device->GetPhysicalDevice().GetEnabledFeatures();
}