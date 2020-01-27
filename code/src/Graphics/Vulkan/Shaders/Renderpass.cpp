#include "Graphics/Vulkan/Shaders/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Renderpass::Renderpass(LogicalDevice & device)
	: Asset(true), device(&device), hndl(nullptr), ownsShaders(false), usesDependency(false),
	PreCreate("RenderpassPreCreate"), PostCreate("RenderpassPostCreate")
{}

Pu::Renderpass::Renderpass(LogicalDevice & device, std::initializer_list<std::initializer_list<wstring>> shaderModules)
	: Renderpass(device)
{
	ownsShaders = true;
	subpasses.reserve(shaderModules.size());

	/* Inline load all the shader modules and then start the creation process. */
	for (const std::initializer_list<wstring> &modules : shaderModules)
	{
		vector<Shader*> shaders(modules.size());

		for (const wstring &shader : modules)
		{
			shaders.emplace_back(new Shader(device, shader));
		}

		subpasses.emplace_back(device.GetPhysicalDevice(), shaders);
	}
}

Pu::Renderpass::Renderpass(LogicalDevice & device, Subpass & subpass)
	: Renderpass(device)
{
	subpasses.emplace_back(subpass);
}

Pu::Renderpass::Renderpass(LogicalDevice & device, vector<Subpass>&& subpasses)
	: Renderpass(device)
{
	this->subpasses = std::move(subpasses);
}

Pu::Renderpass::Renderpass(Renderpass && value)
	: Asset(std::move(value)), device(value.device), hndl(value.hndl), ownsShaders(value.ownsShaders),
	outputDependency(value.outputDependency), subpasses(std::move(value.subpasses)), 
	clearValues(std::move(value.clearValues)), usesDependency(value.usesDependency),
	PreCreate(std::move(value.PreCreate)), PostCreate(std::move(value.PostCreate)), layoutHndl(value.layoutHndl), 
	descriptorSetLayouts(std::move(value.descriptorSetLayouts))
{
	value.hndl = nullptr;
	value.layoutHndl = nullptr;
}

Pu::Renderpass & Pu::Renderpass::operator=(Renderpass && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		device = other.device;
		hndl = other.hndl;
		layoutHndl = other.layoutHndl;
		ownsShaders = other.ownsShaders;
		usesDependency = other.usesDependency;
		outputDependency = other.outputDependency;
		subpasses = std::move(other.subpasses);
		clearValues = std::move(other.clearValues);
		PreCreate = std::move(other.PreCreate);
		PostCreate = std::move(other.PostCreate);
		descriptorSetLayouts = std::move(other.descriptorSetLayouts);

		other.hndl = nullptr;
		other.layoutHndl = nullptr;
	}

	return *this;
}

void Pu::Renderpass::Initialize(void)
{
	/* Just call create, but only if we've not loaded the renderpass! */
	if (!(IsLoaded() || ownsShaders)) Create(false);
}

void Pu::Renderpass::AddDependency(PipelineStageFlag srcStage, PipelineStageFlag dstStage, AccessFlag srcAccess, AccessFlag dstAccess, DependencyFlag flag)
{
	usesDependency = true;
	outputDependency.SrcSubpass = static_cast<uint32>(subpasses.size() - 1);
	outputDependency.DstSubpass = SubpassExternal;
	outputDependency.SrcStageMask = srcStage;
	outputDependency.DstStageMask = dstStage;
	outputDependency.SrcAccessMask = srcAccess;
	outputDependency.DstAccessMask = dstAccess;
	outputDependency.DependencyFlags = flag;
}

Pu::Asset & Pu::Renderpass::Duplicate(AssetCache &)
{
	Reference();

	/* The underlying shaders need to be references as well in case they were loaded with the loader. */
	for (const Subpass &subpass : subpasses)
	{
		for (Shader *shader : subpass.GetShaders()) shader->Reference();
	}

	return *this;
}

void Pu::Renderpass::Recreate(void)
{
	if (IsLoaded())
	{
		/* Destroy the old renderpass. */
		MarkAsLoading();
		device->vkDestroyRenderPass(device->hndl, hndl, nullptr);

		/* Only re-create the renderpass handle. */
		PreCreate.Post(*this);
		CreateRenderpass();
		PostCreate.Post(*this);

		MarkAsLoaded();
	}
}

void Pu::Renderpass::Create(bool viaLoader)
{
	/*
	We need to set a subpass indicator for all outputs, this is used for checking later on.
	We also need to set the attachment reference attachment index,
	otherwise we'll have overlapping indices with multiple subpasses.
	*/
	for (uint32 i = 0, start = 0; i < subpasses.size(); i++)
	{
		for (Output &output : subpasses[i].outputs)
		{
			output.subpass = i;
			output.reference.Attachment += start;
		}

		start += static_cast<uint32>(subpasses[i].outputs.size());
	}

	/*
	We call pre-create to give the owner the chance to initialize the subpasses.
	In create we actually create the renderpass.
	After this we call post-create for optional ownder code and then mark it as loaded.
	Marking as loaded should always be the last thing to do for thread safety.
	*/
	PreCreate.Post(*this);

	CreateRenderpass();
	CreateDescriptorSetLayouts();

	PostCreate.Post(*this);
	MarkAsLoaded(viaLoader, L"Renderpass");
}

void Pu::Renderpass::CreateRenderpass(void)
{
	clearValues.clear();

	/* Copy the attachment descriptions from the initial output fields. */
	vector<AttachmentDescription> attachmentDescriptions;
	for (const Subpass &subpass : subpasses)
	{
		for (const Output &output : subpass.outputs)
		{
			attachmentDescriptions.emplace_back(output.description);
			clearValues.emplace_back(output.clear);
		}
	}

	/* Generate the subpass descriptions, make sure that the vectors don't resize. */
	vector<SubpassDescription> subpassDescriptions;
	vector<vector<AttachmentReference>> inputAttachments{ subpasses.size() };
	vector<vector<AttachmentReference>> colorAttachments{ subpasses.size() };
	vector<vector<AttachmentReference>> resolveAttachments{ subpasses.size() };

	for (const Subpass &subpass : subpasses)
	{
		const size_t i = subpassDescriptions.size();
		const AttachmentReference *depthStencil = nullptr;

		/*
		Add all the color, resolve attachments to a temporary vector
		and set the depth/stencil attachment (if needed).
		*/
		for (const Output &output : subpass.outputs)
		{
			if (output.type == OutputUsage::Color) colorAttachments[i].emplace_back(output.reference);
			else if (output.type == OutputUsage::Resolve) resolveAttachments[i].emplace_back(output.reference);
			else if (output.type == OutputUsage::DepthStencil)
			{
				if (depthStencil) Log::Warning("A depth/stencil attachment is already set for subpass %zu, overriding attachment!", i);
				depthStencil = &output.reference;
			}
		}

		/* Add the input attachments from the descriptors. */
		for (const Descriptor &descriptor : subpass.descriptors)
		{
			if (descriptor.GetType() == DescriptorType::InputAttachment)
			{
				const uint32 idx = descriptor.GetInfo().Decorations.Numbers.at(spv::Decoration::InputAttachmentIndex);
				bool handled = false;

				for (const vector<AttachmentReference> &attachments : colorAttachments)
				{
					for (const AttachmentReference &ref : attachments)
					{
						/* The input attachment will have the same index as a previous subpass color attachment. */
						if (ref.Attachment == idx)
						{
							inputAttachments[i].emplace_back(ref);
							handled = true;
							break;
						}
					}

					if (handled) break;
				}
			}
		}

		/* Create the subpass description, TODO: handle preserve attachments. */
		SubpassDescription description{ colorAttachments[i], inputAttachments[i], resolveAttachments[i] };
		description.DepthStencilAttachment = depthStencil;
		subpassDescriptions.emplace_back(description);
	}

	/* Copy the dependencies from the subpasses. */
	vector<SubpassDependency> dependencies;
	for (uint32 i = 0; i < subpasses.size(); i++)
	{
		const Subpass &subpass = subpasses[i];
		if (subpass.dependencyUsed)
		{
			dependencies.emplace_back(subpass.dependency);
			dependencies.back().DstSubpass = i;

			/* We must set the source (previous) and destination (current) subpass ourselves. */
			if (i == 0) dependencies.back().SrcSubpass = SubpassExternal;
			else dependencies.back().SrcSubpass = i - 1;
		}
	}

	/* Only add the external output dependency if the user created one. */
	if (usesDependency) dependencies.emplace_back(outputDependency);

	/* Create the renderpass. */
	const RenderPassCreateInfo createInfo(attachmentDescriptions, subpassDescriptions, dependencies);
	VK_VALIDATE(device->vkCreateRenderPass(device->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateRenderPass);
}

void Pu::Renderpass::CreateDescriptorSetLayouts(void)
{
	/*
	We want to precreate a list of all descriptor sets with their specific bindings.
	For each of these sets we need to create a layout handle.
	*/
	std::map<uint32, vector<DescriptorSetLayoutBinding>> layoutBindings;
	vector<PushConstantRange> constRanges;

	for (const Subpass &subpass : subpasses)
	{
		/* Get all the descriptor sets from the subpass. */
		for (const Descriptor &descriptor : subpass.descriptors)
		{
			/* Check if the set is already in the list. */
			decltype(layoutBindings)::iterator it = layoutBindings.find(descriptor.set);
			if (it != layoutBindings.end())
			{
				/* Descriptors might have the same binding (multiple in a uniform buffer) we should theat this as one descriptor. */
				vector<DescriptorSetLayoutBinding>::iterator it2 = it->second.iteratorOf([&descriptor](const DescriptorSetLayoutBinding &cur) { return cur.Binding == descriptor.GetBinding(); });
				if (it2 == it->second.end()) it->second.emplace_back(descriptor.layoutBinding);
			}
			else
			{
				/* Just add the descriptor to the list with its parent set. */
				vector<DescriptorSetLayoutBinding> value = { descriptor.layoutBinding };
				layoutBindings.emplace(descriptor.set, std::move(value));
			}
		}

		/* Just add the push constant ranges to the list. */
		for (const PushConstant &pushConstant : subpass.pushConstants)
		{
			vector<PushConstantRange>::iterator it = constRanges.iteratorOf([&pushConstant](const PushConstantRange &cur) { return cur.StageFlags == pushConstant.range.StageFlags; });
			if (it != constRanges.end()) it->Size += static_cast<uint32>(pushConstant.GetSize());
			else constRanges.emplace_back(pushConstant.range);
		}
	}

	/* Create the descriptor sets. */
	descriptorSetLayouts.resize(layoutBindings.size());
	size_t i = 0;
	for (const auto &[set, bindings] : layoutBindings)
	{
		const DescriptorSetLayoutCreateInfo createInfo(bindings);
		VK_VALIDATE(device->vkCreateDescriptorSetLayout(device->hndl, &createInfo, nullptr, &descriptorSetLayouts[i++]), PFN_vkCreateDescriptorSetLayout);
	}

	/* Create the pipeline layout. */
	const PipelineLayoutCreateInfo layoutCreateInfo(descriptorSetLayouts, constRanges);
	VK_VALIDATE(device->vkCreatePipelineLayout(device->hndl, &layoutCreateInfo, nullptr, &layoutHndl), PFN_vkCreatePipelineLayout);
}

void Pu::Renderpass::Destroy(void)
{
	if (hndl) device->vkDestroyRenderPass(device->hndl, hndl, nullptr);
	if (layoutHndl) device->vkDestroyPipelineLayout(device->hndl, layoutHndl, nullptr);

	/* This means we loaded the shaders inline, which means we need to free them. */
	if (ownsShaders)
	{
		for (const Subpass &subpass : subpasses)
		{
			for (const Shader *shader : subpass.GetShaders()) delete shader;
		}
	}

	/* Release the descriptor set layouts. */
	for (DescriptorSetLayoutHndl cur : descriptorSetLayouts) device->vkDestroyDescriptorSetLayout(device->hndl, cur, nullptr);
}

Pu::Renderpass::LoadTask::LoadTask(Renderpass & result, const vector<std::tuple<size_t, size_t, wstring>>& toLoad)
	: renderpass(result)
{
	children.reserve(toLoad.size());
	for (const auto &[subpass, idx, path] : toLoad)
	{
		children.emplace_back(new Shader::LoadTask(*renderpass.subpasses[subpass].shaders[idx], path));
	}
}

Pu::Task::Result Pu::Renderpass::LoadTask::Execute(void)
{
	for (Shader::LoadTask *task : children)
	{
		task->SetParent(*this);
		scheduler->Spawn(*task);
	}

	return Result::CustomWait();
}

bool Pu::Renderpass::LoadTask::ShouldContinue(void) const
{
	/* 
	We want to check if all shaders are loaded before we continue. 
	We can have a race if two renderpasses use the same shader and are loaded at the same time.
	*/
	for (const Subpass &subpass : renderpass.subpasses)
	{
		for (const Shader *shader : subpass.shaders)
		{
			if (!shader->IsLoaded()) return false;
		}
	}

	return Task::ShouldContinue();
}

Pu::Task::Result Pu::Renderpass::LoadTask::Continue(void)
{
	/* Make sure that the subpasses are compatible and initialized. */
	for (Subpass &subpass : renderpass.subpasses) subpass.Link(renderpass.device->GetPhysicalDevice());

	/* Delete the underlying shader load tasks, create the renderpass and finally allow the scheduler to delete this task. */
	for (const Shader::LoadTask *task : children) delete task;
	renderpass.Create(true);
	return Result::AutoDelete();
}