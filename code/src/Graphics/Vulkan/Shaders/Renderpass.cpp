#include "Graphics/Vulkan/Shaders/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

Pu::Renderpass::Renderpass(LogicalDevice & device)
	: Asset(true), device(&device), hndl(nullptr), ownsShaders(false),
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

	Create(false);
}

Pu::Renderpass::Renderpass(LogicalDevice & device, vector<Subpass>&& subpasses)
	: Renderpass(device)
{
	this->subpasses = std::move(subpasses);
	Create(false);
}

Pu::Renderpass::Renderpass(Renderpass && value)
	: Asset(std::move(value)), device(value.device), hndl(value.hndl), ownsShaders(value.ownsShaders),
	subpasses(std::move(value.subpasses)), clearValues(std::move(value.clearValues)),
	PreCreate(std::move(value.PreCreate)), PostCreate(std::move(value.PostCreate)), layoutHndl(value.layoutHndl),
	inputAttachments(std::move(value.inputAttachments)), depthStencilAttachments(std::move(value.depthStencilAttachments)),
	preserveAttachments(std::move(value.preserveAttachments)), descriptorSetLayouts(std::move(value.descriptorSetLayouts))
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
		subpasses = std::move(other.subpasses);
		clearValues = std::move(other.clearValues);
		PreCreate = std::move(other.PreCreate);
		PostCreate = std::move(other.PostCreate);
		inputAttachments = std::move(other.inputAttachments);
		depthStencilAttachments = std::move(other.depthStencilAttachments);
		preserveAttachments = std::move(other.preserveAttachments);
		descriptorSetLayouts = std::move(other.descriptorSetLayouts);

		other.hndl = nullptr;
		other.layoutHndl = nullptr;
	}

	return *this;
}

void Pu::Renderpass::Preserve(const Output & field, uint32 subpass)
{
	/* Check if the specified subpass already has preserve attachments. */
	decltype(preserveAttachments)::iterator it = preserveAttachments.find(subpass);
	if (it != preserveAttachments.end())
	{
		/* Add the attachment to the pre-existing list. */
		it->second.emplace_back(field.reference.Attachment, field.reference.Layout);
	}
	else
	{
		/* Just add a new vector (with the reference) to the list. */
		vector<AttachmentReference> value = { AttachmentReference(field.reference.Attachment, field.reference.Layout) };
		preserveAttachments.emplace(subpass, value);
	}
}

void Pu::Renderpass::SetAsInput(const Output & field, ImageLayout layout, uint32 subpass)
{
	/* Only color and depth/stencil attachments can be passed to this method. */
	if (field.type == OutputUsage::Color)
	{
		/* Check if the subpass already exists. */
		decltype(inputAttachments)::iterator it = inputAttachments.find(subpass);
		if (it != inputAttachments.end())
		{
			/* Add the attachment to the pre-existing list. */
			it->second.emplace_back(field.reference.Attachment, layout);
		}
		else
		{
			/* Just add a new vector (with the reference) to the list. */
			vector<AttachmentReference> value = { AttachmentReference(field.reference.Attachment, layout) };
			inputAttachments.emplace(subpass, value);
		}
	}
	else if (field.type == OutputUsage::DepthStencil)
	{
		/* Check if the subpass already exists. */
		decltype(depthStencilAttachments)::iterator it = depthStencilAttachments.find(subpass);
		if (it == depthStencilAttachments.end())
		{
			/* Check if we can add a depth/stencil attachment to the specified subpass. */
			for (const Output &output : subpasses[subpass].outputs)
			{
				if (output.type == OutputUsage::DepthStencil)
				{
					Log::Error("Subpass %u already defined a depth/stencil attachment!", subpass);
					return;
				}
			}

			depthStencilAttachments.emplace(subpass, AttachmentReference(field.reference.Attachment, layout));
		}
		else Log::Error("Subpass %u already has a depth/stencil attachment defined!", subpass);
	}
	else Log::Error("Cannot use '%s' (%s attachment) as an input!", field.GetInfo().Name.c_str(), to_string(field.type));
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
	if (viaLoader) PreCreate.Post(*this);

	CreateRenderpass();
	CreateDescriptorSetLayouts();

	if (viaLoader) PostCreate.Post(*this);
	MarkAsLoaded(viaLoader, L"Renderpass");
}

void Pu::Renderpass::CreateRenderpass(void)
{
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

	/* Generate the subpass descriptions. */
	uint32 i = 0;
	size_t j = 0;
	vector<SubpassDescription> subpassDescriptions;
	vector<AttachmentReference> colorAttachments;

	/* Make sure the vector doesn't resize. */
	colorAttachments.reserve(attachmentDescriptions.size());

	for (const Subpass &subpass : subpasses)
	{
		SubpassDescription description;

		/* Add all the input attachments to the description. */
		decltype(inputAttachments)::const_iterator it = inputAttachments.find(i);
		if (it != inputAttachments.end())
		{
			description.InputAttachmentCount = static_cast<uint32>(it->second.size());
			description.InputAttachments = it->second.data();
		}

		/*
		Add all the color and resolve attachments to a temporary vector
		and set the depth/stencil attachment (if needed).
		*/
		for (const Output &output : subpass.outputs)
		{
			if (output.type == OutputUsage::Color) colorAttachments.emplace_back(output.reference);
			// TODO: handle resolve attachments.
			else if (output.type == OutputUsage::DepthStencil) description.DepthStencilAttachment = &output.reference;
		}

		description.ColorAttachmentCount = static_cast<uint32>(colorAttachments.size() - j);
		description.ColorAttachments = colorAttachments.data() + j;
		// TODO: handle resolve attachments.

		/* Set the depth/stencil attachment (if needed), a check for multiple has already occured. */
		decltype(depthStencilAttachments)::const_iterator it2 = depthStencilAttachments.find(i);
		if (it2 != depthStencilAttachments.end()) description.DepthStencilAttachment = &it2->second;

		/* Set the preserve attachments. */
		decltype(preserveAttachments)::const_iterator it3 = preserveAttachments.find(i);
		if (it3 != preserveAttachments.end())
		{
			description.PreserveAttachmentCount = static_cast<uint32>(it3->second.size());
			description.PreserveAttachments = it3->second.data();
		}

		i++;
		j += description.ColorAttachmentCount;
		subpassDescriptions.emplace_back(description);
	}

	/* Copy the dependencies from the subpasses. */
	vector<SubpassDependency> dependencies;
	for (const Subpass &subpass : subpasses)
	{
		if (subpass.dependencyUsed) dependencies.emplace_back(subpass.dependency);
	}

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

	for (const Subpass &subpass : subpasses)
	{
		for (const Descriptor &descriptor : subpass.descriptors)
		{
			/* Check if the set is already in the list. */
			decltype(layoutBindings)::iterator it = layoutBindings.find(descriptor.set);
			if (it != layoutBindings.end())
			{
				/* Either add the descriptor count to the binding or add a new binding to the set. */
				vector<DescriptorSetLayoutBinding>::iterator it2 = it->second.iteratorOf([&descriptor](const DescriptorSetLayoutBinding &cur) { return cur.Binding == descriptor.GetBinding(); });
				if (it2 != it->second.end()) it2->DescriptorCount += descriptor.layoutBinding.DescriptorCount;
				else it->second.emplace_back(descriptor.layoutBinding);
			}
			else
			{
				/* Just add the descriptor to the list with its parent set. */
				vector<DescriptorSetLayoutBinding> value = { descriptor.layoutBinding };
				layoutBindings.emplace(descriptor.set, std::move(value));
			}
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
	const PipelineLayoutCreateInfo layoutCreateInfo(descriptorSetLayouts);
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