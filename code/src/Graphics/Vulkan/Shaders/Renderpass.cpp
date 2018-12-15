#include "Graphics/Vulkan/Shaders/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Renderpass::Renderpass(LogicalDevice & device)
	: device(device), hndl(nullptr), loaded(false), usable(false),
	OnAttachmentLink("RenderpassOnAttachmentLink")
{}

Pu::Renderpass::Renderpass(LogicalDevice & device, vector<Subpass>&& subpasses)
	: device(device), subpasses(std::move(subpasses)), usable(false),
	OnAttachmentLink("RenderpassOnAttachmentLink")
{
	Link();
}

Pu::Renderpass::Renderpass(Renderpass && value)
	: device(value.device), hndl(value.hndl), subpasses(std::move(value.subpasses)),
	loaded(value.IsLoaded()), usable(value.usable), OnAttachmentLink(std::move(value.OnAttachmentLink))
{
	value.hndl = nullptr;
}

Pu::Renderpass & Pu::Renderpass::operator=(Renderpass && other)
{
	if (this != &other)
	{
		device = std::move(other.device);
		hndl = other.hndl;
		subpasses = std::move(other.subpasses);
		loaded.store(other.IsLoaded());
		usable = other.usable;
		OnAttachmentLink = std::move(other.OnAttachmentLink);

		other.hndl = nullptr;
		other.loaded.store(false);
		other.usable = false;
	}

	return *this;
}

Pu::Output & Pu::Renderpass::GetOutput(const string & name)
{
	for (Output &cur : outputs)
	{
		if (name == cur.Info.Name) return cur;
	}

	Log::Fatal("Unable to find output field '%s'!", name.c_str());
}

void Pu::Renderpass::Link(void)
{
	/* Start by sorting all subpasses on their invokation time in the Vulkan pipeline (Vertex -> Tessellation -> Geometry -> Fragment). */
	std::sort(subpasses.begin(), subpasses.end(), [](const Subpass &a, const Subpass &b)
	{
		return _CrtEnum2Int(a.GetType()) < _CrtEnum2Int(b.GetType());
	});

	/* Check if the supplied shader modules can be liked together. */
	for (size_t i = 0, j = 1; j < subpasses.size(); i++, j++)
	{
		if (!CheckIO(subpasses[i], subpasses[j]))
		{
			/* Shader is done loading but failed linking. */
			Log::Error("Unable to link %s shader to %s shader!", to_string(subpasses[i].GetType()), to_string(subpasses[j].GetType()));
			LinkFailed();
			return;
		}
	}

	/* Save a variable for how many attachment references are made. */
	uint32 referenceCnt = 0;

	/* Load all outputs. */
	const Subpass &outputPass = subpasses.back();
	for (size_t i = 0; i < outputPass.GetFieldCount(); i++)
	{
		const FieldInfo &info = outputPass.GetField(i);
		if (info.Storage == spv::StorageClass::Output) outputs.push_back(Output(info, referenceCnt++));
	}

	// TODO: load all attributes and uniforms

	/* Set all attachment references for the subpass. */
	vector<AttachmentReference> colorAttachments, resolveAttachments, depthStencilAttachments, preserveAttachments;
	for (const Output &cur : outputs)
	{
		switch (cur.type)
		{
		case (OutputUsage::Color):
			colorAttachments.push_back(cur.reference);
			if (cur.resolve) resolveAttachments.push_back(cur.reference);
			break;
		case (OutputUsage::DepthStencil):
			depthStencilAttachments.push_back(cur.reference);
			break;
		case (OutputUsage::Preserve):
			preserveAttachments.push_back(cur.reference);
			break;
		}
	}

	vector<SubpassDescription> subpassDescriptions;
	vector<AttachmentDescription> attachmentDescriptions;

	// TODO: actually add descriptions based on shader information.
	// Temporary block.
	/*--------------------------------------------------------------------------------------------------------------*/
	SubpassDescription temp;
	temp.ColorAttachmentCount = colorAttachments.size();
	temp.ColorAttachments = colorAttachments.data();
	subpassDescriptions.push_back(temp);
	/*--------------------------------------------------------------------------------------------------------------*/

	/* Give user the opertunity to set descriptions for all used fields. */
	OnAttachmentLink.Post(*this, EventArgs());

	/* Copy descriptions from the outputs to the attachmentDescription buffer. */
	for (const Output &output : outputs)
	{
		attachmentDescriptions.emplace_back(output.description);
	}

	/* Link the subpasses into a render pass. */
	RenderPassCreateInfo createInfo(attachmentDescriptions, subpassDescriptions);
	VK_VALIDATE(device.vkCreateRenderPass(device.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateRenderPass);

	LinkSucceeded();
}

bool Pu::Renderpass::CheckIO(const Subpass & a, const Subpass & b) const
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
			if (bInfo.Storage == spv::StorageClass::Input && bInfo.Location == aInfo.Location)
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
				checked.push_back(j);
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

void Pu::Renderpass::LinkSucceeded(void)
{
	usable = true;
	loaded.store(true);

#ifdef _DEBUG
	string modules;
	for (const Subpass &pass : subpasses)
	{
		modules += pass.GetName();
		if (pass.GetType() != ShaderStageFlag::Fragment) modules += " -> ";
	}

	Log::Verbose("Successfully linked render pass: %s.", modules.c_str());
#endif
}

void Pu::Renderpass::LinkFailed(void)
{
	usable = false;
	loaded.store(true);
}

Pu::Renderpass::LoadTask::LoadTask(Renderpass & result, std::initializer_list<const char*> subpasses)
	: result(result), paths(std::move(subpasses))
{}

Pu::Task::Result Pu::Renderpass::LoadTask::Execute(void)
{
	/* We need to pre add the subpassed because vector resizing created bad refrences. */
	size_t i = 0;
	for (; i < paths.size(); i++) result.subpasses.emplace_back(result.device);

	i = 0;
	for (const char *cur : paths)
	{
		/* Create the subpass loading task */
		Subpass::LoadTask *task = new Subpass::LoadTask(result.subpasses[i++], cur);
		task->SetParent(*this);
		children.emplace_back(task);

		/* Give the task to the scheduler. */
		scheduler->Spawn(*task);
	}

	return Result();
}

Pu::Task::Result Pu::Renderpass::LoadTask::Continue(void)
{
	/* Make sure that all subpasses are loaded on debug mode. */
#ifdef _DEBUG
	for (const Subpass &cur : result.subpasses)
	{
		if (!cur.IsLoaded()) Log::Error("Not every subpass has completed loading!");
	}
#endif

	/* Delete the child tasks. */
	for (Subpass::LoadTask *subTask : children)
	{
		delete subTask;
	}

	/* Perform linking and return. */
	result.Link();
	return Result();
}