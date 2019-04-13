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

	/*
	Add it to the list and return it for direct use, the reference will always be 1 as we cannot have multiple depth stencil targets.
	We will not throw yet if the user creates multiple depth stencil targets, we'll throw on finalize.
	*/
	outputs.emplace_back(Output(info, 1, OutputUsage::DepthStencil));
	return outputs[outputs.size() - 1];
}

/* The compiler complains that not all codepaths return a value, but Log::Fatal will always throw. */
#pragma warning (push)
#pragma warning (disable:4715)
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
#pragma warning(pop)

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
	for (const FieldInfo &info : shaders.front().get().fields)
	{
		if (info.Storage == spv::StorageClass::Input) attributes.emplace_back(Attribute(info));
	}

	/* Load all uniforms. */
	for (const Shader &pass : shaders)
	{
		for (const FieldInfo &info : pass.fields)
		{
			if (info.Storage == spv::StorageClass::UniformConstant || info.Storage == spv::StorageClass::Uniform)
			{
				uniforms.emplace_back(Uniform(info, pass.GetType()));
			}
		}
	}

	/* Save a variable for how many attachment references are made and load the outputs. */
	uint32 referenceCnt = 0;
	for (const FieldInfo &info : shaders.back().get().fields)
	{
		if (info.Storage == spv::StorageClass::Output) outputs.emplace_back(Output(info, referenceCnt++, OutputUsage::Color));
	}
}

void Pu::Renderpass::Finalize(bool linkedViaLoader)
{
	/* Set all attachment references for the subpass. */
	vector<AttachmentReference> colorAttachments, resolveAttachments, preserveAttachments;
	const AttachmentReference *depthStencil = nullptr;

	for (const Output &cur : outputs)
	{
		switch (cur.type)
		{
		case (OutputUsage::Color):
			colorAttachments.emplace_back(cur.reference);
			if (cur.resolve) resolveAttachments.emplace_back(cur.reference);
			break;
		case (OutputUsage::DepthStencil):
			if (depthStencil) Log::Error("Multiple depth/stencil attachments were added, only the firs one will be used!");
			else depthStencil = &cur.reference;
			break;
		case (OutputUsage::Preserve):
			preserveAttachments.emplace_back(cur.reference);
			break;
		}
	}

	/* We onyl support one subpass per renderpass as they're thus far not often used on non-mobile platforms. */
	SubpassDescription subpass(colorAttachments, depthStencil, preserveAttachments);

	/* Copy descriptions from the outputs to the attachmentDescription buffer. */
	vector<AttachmentDescription> attachmentDescriptions;
	for (const Output &output : outputs)
	{
		attachmentDescriptions.emplace_back(output.description);
		clearValues.emplace_back(output.clear);
	}

	/* Link the subpasses into a render pass. */
	RenderPassCreateInfo createInfo(attachmentDescriptions, subpass, dependencies);
	VK_VALIDATE(device.vkCreateRenderPass(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateRenderPass);
	LinkSucceeded(linkedViaLoader);
}

bool Pu::Renderpass::CheckIO(const Shader & a, const Shader & b) const
{
	bool result = true;
	vector<spv::Word> linkedLocations;

	/* Check all field in the first module. */
	for (const FieldInfo &aInfo : a.fields)
	{
		/* Output fields need to be linked to input fields in the next pass and any other can be ignored. */
		if (aInfo.Storage != spv::StorageClass::Output) continue;

		bool found = false;
		for (const FieldInfo &bInfo : b.fields)
		{
			/* We can ignore non-input fields. */
			if (bInfo.Storage != spv::StorageClass::Input) continue;
			if (aInfo.GetLocation() == bInfo.GetLocation())
			{
				/* Check if the types differ. */
				if (aInfo.Type != bInfo.Type)
				{
					Log::Error("The type of '%s' (%s) does't match '%s' (%s)!", aInfo.Name.c_str(), a.GetName().c_str(), bInfo.Name.c_str(), b.GetName().c_str());
					result = false;
					continue;
				}

				/* Check if the input is linked to multiple outputs. */
				if (linkedLocations.contains(bInfo.GetLocation()))
				{
					Log::Error("Multiple output fields are using '%s' (%s) as an input field!", bInfo.Name.c_str(), b.GetName().c_str());
					result = false;
					continue;
				}

				/* We've validated this location so add it to the list.  */
				linkedLocations.emplace_back(bInfo.GetLocation());
				found = true;
			}
		}

		/* Log an error if a link could not be made. */
		if (!found)
		{
			Log::Error("Unable top find matching field in %s shader (%s) for %s in %s shader (%s)!", to_string(b.GetType()), b.GetName().c_str(), aInfo.Name.c_str(), to_string(a.GetType()), b.GetName().c_str());
			result = false;
		}
	}

	/* Check if any input fields of b have gone unset. */
	for (const FieldInfo &info : b.fields)
	{
		if (info.Storage != spv::StorageClass::Input) continue;

		if (!linkedLocations.contains(info.GetLocation()))
		{
			Log::Error("%s in %s shader (%s) was not set by previous %s shader (%s)!", info.Name.c_str(), to_string(b.GetType()), b.GetName().c_str(), to_string(a.GetType()), b.GetName().c_str());
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