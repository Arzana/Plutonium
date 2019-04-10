#include "Graphics/Vulkan/Shaders/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Renderpass::Renderpass(LogicalDevice & device)
	: Asset(true), device(device), hndl(nullptr), usable(false),
	OnLinkCompleted("RenderpassOnLinkCompleted")
{}

Pu::Renderpass::Renderpass(LogicalDevice & device, vector<std::reference_wrapper<Shader>>&& subpasses)
	: Asset(true), device(device), shaders(std::move(subpasses)), usable(false),
	OnLinkCompleted("RenderpassOnLinkCompleted")
{
	SetHash(std::hash<wstring>{}(subpasses.select<wstring>([](const Shader &cur) { return cur.GetName(); })));
	Link(false);
}

Pu::Renderpass::Renderpass(Renderpass && value)
	: Asset(std::move(value)), device(value.device), hndl(value.hndl), shaders(std::move(value.shaders)),
	usable(value.usable), OnLinkCompleted(std::move(value.OnLinkCompleted)),
	outputs(std::move(value.outputs)), clearValues(std::move(value.clearValues)),
	dependencies(std::move(value.dependencies)), attributes(std::move(value.attributes)),
	uniforms(std::move(value.uniforms))
{
	value.hndl = nullptr;
	value.usable = false;
}

Pu::Renderpass & Pu::Renderpass::operator=(Renderpass && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		device = std::move(other.device);
		hndl = other.hndl;
		shaders = std::move(other.shaders);
		usable = other.usable;
		OnLinkCompleted = std::move(other.OnLinkCompleted);
		outputs = std::move(other.outputs);
		clearValues = std::move(other.clearValues);
		dependencies = std::move(other.dependencies);
		attributes = std::move(other.attributes);
		uniforms = std::move(other.uniforms);

		other.hndl = nullptr;
		other.usable = false;
	}

	return *this;
}

Pu::Output & Pu::Renderpass::AddDepthStencil(void)
{
	/* Create some default info for the field. */
	FieldType type(ComponentType::Float, SizeType::Scalar);
	FieldInfo info(0, "DepthStencil", std::move(type), spv::StorageClass::Output, Decoration());

	/* Add it to the list and return it for direct use, the reference will always be 1 as we cannot have multiple depth stencil targets. */
	outputs.emplace_back(Output(info, 1, OutputUsage::DepthStencil));
	return outputs[outputs.size() - 1];
}

Pu::Output & Pu::Renderpass::GetOutput(const string & name)
{
	for (Output &cur : outputs)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find output field '%s'!", name.c_str());
}

const Pu::Output & Pu::Renderpass::GetOutput(const string & name) const
{
	for (const Output &cur : outputs)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find output field '%s'!", name.c_str());
}

Pu::Attribute & Pu::Renderpass::GetAttribute(const string & name)
{
	for (Attribute &cur : attributes)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find attribute field '%s'!", name.c_str());
}

const Pu::Attribute & Pu::Renderpass::GetAttribute(const string & name) const
{
	for (const Attribute &cur : attributes)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find attribute field '%s'!", name.c_str());
}

Pu::Uniform & Pu::Renderpass::GetUniform(const string & name)
{
	for (Uniform &cur : uniforms)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find uniform field '%s'!", name.c_str());
}

const Pu::Uniform & Pu::Renderpass::GetUniform(const string & name) const
{
	for (const Uniform &cur : uniforms)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find uniform field '%s'!", name.c_str());
}

Pu::Asset & Pu::Renderpass::Duplicate(AssetCache &)
{
	Reference();
	for (Shader &cur : shaders) cur.Reference();
	return *this;
}

void Pu::Renderpass::Link(bool linkedViaLoader)
{
	/* Start by sorting all subpasses on their invokation time in the Vulkan pipeline (Vertex -> Tessellation -> Geometry -> Fragment). */
	std::sort(shaders.begin(), shaders.end(), [](const Shader &a, const Shader &b)
	{
		return _CrtEnum2Int(a.GetType()) < _CrtEnum2Int(b.GetType());
	});

	/* Check if the supplied shader modules can be linked together. */
	for (size_t i = 0, j = 1; j < shaders.size(); i++, j++)
	{
		if (!CheckIO(shaders[i], shaders[j]))
		{
			/* Shader is done loading but failed linking. */
			Log::Error("Unable to link %s shader to %s shader!", to_string(shaders[i].get().GetType()), to_string(shaders[j].get().GetType()));
			LinkFailed(linkedViaLoader);
			return;
		}
	}

	/* Load all fields that need to be accessed outside the shaders. */
	LoadFields();

	/* Give user the opertunity to set descriptions for all used fields. */
	OnLinkCompleted.Post(*this);

	/* Finalize the render pass. */
	Finalize(linkedViaLoader);
}

void Pu::Renderpass::LoadFields(void)
{
	/* Load all attributes. */
	const Shader &inputPass = shaders.front();
	for (size_t i = 0; i < inputPass.GetFieldCount(); i++)
	{
		const FieldInfo &info = inputPass.GetField(i);
		if (info.Storage == spv::StorageClass::Input) attributes.emplace_back(Attribute(info));
	}

	/* Load all uniforms. */
	for (const Shader &pass : shaders)
	{
		for (size_t i = 0; i < pass.GetFieldCount(); i++)
		{
			const FieldInfo &info = pass.GetField(i);
			if (info.Storage == spv::StorageClass::UniformConstant || info.Storage == spv::StorageClass::Uniform)
			{
				uniforms.emplace_back(Uniform(info, pass.GetType()));
			}
		}
	}

	/* Save a variable for how many attachment references are made. */
	uint32 referenceCnt = 0;

	/* Load all outputs. */
	const Shader &outputPass = shaders.back();
	for (size_t i = 0; i < outputPass.GetFieldCount(); i++)
	{
		const FieldInfo &info = outputPass.GetField(i);
		if (info.Storage == spv::StorageClass::Output) outputs.emplace_back(Output(info, referenceCnt++, OutputUsage::Color));
	}
}

void Pu::Renderpass::Finalize(bool linkedViaLoader)
{
	/* Set all attachment references for the subpass. */
	vector<AttachmentReference> colorAttachments, resolveAttachments, depthStencilAttachments, preserveAttachments;
	for (const Output &cur : outputs)
	{
		switch (cur.type)
		{
		case (OutputUsage::Color):
			colorAttachments.emplace_back(cur.reference);
			if (cur.resolve) resolveAttachments.emplace_back(cur.reference);
			break;
		case (OutputUsage::DepthStencil):
			depthStencilAttachments.emplace_back(cur.reference);
			break;
		case (OutputUsage::Preserve):
			preserveAttachments.emplace_back(cur.reference);
			break;
		}
	}

	/* Make sure not too many depth/stencil attachment were created. */
	if (depthStencilAttachments.size() > 1) Log::Error("Multiple depth stencil attachment were added to renderpass, only the first one will be used!");

	vector<SubpassDescription> subpassDescriptions;
	vector<AttachmentDescription> attachmentDescriptions;

	// TODO: actually add descriptions based on shader information.
	// Temporary block.
	/*--------------------------------------------------------------------------------------------------------------*/
	SubpassDescription temp;
	temp.ColorAttachmentCount = static_cast<uint32>(colorAttachments.size());
	temp.ColorAttachments = colorAttachments.data();
	temp.DepthStencilAttachment = depthStencilAttachments.data();
	temp.PreserveAttachmentCount = static_cast<uint32>(preserveAttachments.size());
	temp.PreserveAttachments = preserveAttachments.data();
	subpassDescriptions.emplace_back(temp);
	/*--------------------------------------------------------------------------------------------------------------*/

	/* Copy descriptions from the outputs to the attachmentDescription buffer. */
	for (const Output &output : outputs)
	{
		attachmentDescriptions.emplace_back(output.description);
		clearValues.emplace_back(output.clear);
	}

	/* Link the subpasses into a render pass. */
	RenderPassCreateInfo createInfo(attachmentDescriptions, subpassDescriptions);
	createInfo.DependencyCount = static_cast<uint32>(dependencies.size());
	createInfo.Dependencies = dependencies.data();
	VK_VALIDATE(device.vkCreateRenderPass(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateRenderPass);
	LinkSucceeded(linkedViaLoader);
}

bool Pu::Renderpass::CheckIO(const Shader & a, const Shader & b) const
{
	bool result = true;
	vector<size_t> checked;

	/* Check all field in the first module. */
	for (size_t i = 0, j = 0; i < a.GetFieldCount(); i++)
	{
		/* Only handle output fields. */
		const FieldInfo &aInfo = a.GetField(i);
		if (aInfo.Storage != spv::StorageClass::Output) continue;

		/* Attempt to find the matching field. */
		for (j = 0; j < b.GetFieldCount(); j++)
		{
			const FieldInfo &bInfo = b.GetField(j);
			if (bInfo.Storage == spv::StorageClass::Input && bInfo.GetLocation() == aInfo.GetLocation())
			{
				/* Raise if the types of the fields don't match. */
				if (aInfo.Type != bInfo.Type)
				{
					Log::Error("Output field %s's type doesn't match input field %s's type!", aInfo.Name.c_str(), bInfo.Name.c_str());
					result = false;
					continue;
				}

				/* Raise if the input field is used by multiple outputs. */
				if (checked.contains(j))
				{
					Log::Error("Multiple output fields are using %s as an input field!", bInfo.Name.c_str());
					result = false;
					continue;
				}

				/* Break to prevent j++. */
				checked.emplace_back(j);
				break;
			}
		}

		/* Raise if no matching field could be found. */
		if (j >= b.GetFieldCount())
		{
			Log::Error("Unable to find matching field in %s shader for %s shader's field %s!", to_string(b.GetType()), to_string(a.GetType()), aInfo.Name.c_str());
			result = false;
		}
	}

	/* Check if any input fields of b have gone unset. */
	for (size_t i = 0; i < b.GetFieldCount(); i++)
	{
		const FieldInfo &bInfo = b.GetField(i);
		if (bInfo.Storage != spv::StorageClass::Input) continue;

		if (!checked.contains(i))
		{
			Log::Error("Input field %s was not set by previous shader!", bInfo.Name.c_str());
			result = false;
		}
	}

	return result;
}

void Pu::Renderpass::LinkSucceeded(bool linkedViaLoader)
{
	usable = true;
	MarkAsLoaded(linkedViaLoader, L"Renderpass");

#ifdef _DEBUG
	wstring modules;
	for (const Shader &pass : shaders)
	{
		modules += pass.GetName();
		if (pass.GetType() != ShaderStageFlag::Fragment) modules += L" -> ";
	}

	Log::Verbose("Successfully linked render pass: %ls.", modules.c_str());
#endif
}

void Pu::Renderpass::LinkFailed(bool linkedViaLoader)
{
	usable = false;
	MarkAsLoaded(linkedViaLoader, L"Renderpass");
}

void Pu::Renderpass::Destroy(void)
{
	if (hndl) device.vkDestroyRenderPass(device.hndl, hndl, nullptr);
}

Pu::Renderpass::LoadTask::LoadTask(Renderpass & result, const vector<std::tuple<size_t, wstring>>& toLoad)
	: result(result)
{
	vector<wstring> hashParams(toLoad.size());

	/* Create new load tasks for the to load subpasses. */
	for (auto[idx, path] : toLoad)
	{
		hashParams.emplace_back(path);
		children.emplace_back(new Shader::LoadTask(result.shaders[idx], path));
	}

	result.SetHash(std::hash<wstring>{}(hashParams));
}

Pu::Task::Result Pu::Renderpass::LoadTask::Execute(void)
{
	/* Spawn all still required load tasks. */
	for (Shader::LoadTask *task : children)
	{
		task->SetParent(*this);
		scheduler->Spawn(*task);
	}

	return Result::Default();
}

Pu::Task::Result Pu::Renderpass::LoadTask::Continue(void)
{
	/* Make sure that all subpasses are loaded on debug mode. */
#ifdef _DEBUG
	for (const Shader &cur : result.shaders)
	{
		if (!cur.IsLoaded()) Log::Error("Not every subpass has completed loading!");
	}
#endif

	/* Delete the child tasks. */
	for (Shader::LoadTask *subTask : children)
	{
		delete subTask;
	}

	/* Perform linking (this should always be called from the loader) and return. */
	result.Link(true);
	return Result::Default();
}