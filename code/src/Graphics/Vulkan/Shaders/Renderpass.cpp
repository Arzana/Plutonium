#include "Graphics/Vulkan/Shaders/Renderpass.h"
#include "Core/Threading/Tasks/Scheduler.h"

#ifdef _DEBUG
#include <set>
#endif

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

		subpasses.emplace_back(device, shaders);
	}
}

Pu::Renderpass::Renderpass(LogicalDevice & device, Subpass && subpass)
	: Renderpass(device)
{
	subpasses.emplace_back(std::move(subpass));
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
	PreCreate(std::move(value.PreCreate)), PostCreate(std::move(value.PostCreate))
{
	value.hndl = nullptr;
}

Pu::Renderpass & Pu::Renderpass::operator=(Renderpass && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		device = other.device;
		hndl = other.hndl;
		ownsShaders = other.ownsShaders;
		usesDependency = other.usesDependency;
		outputDependency = other.outputDependency;
		subpasses = std::move(other.subpasses);
		clearValues = std::move(other.clearValues);
		PreCreate = std::move(other.PreCreate);
		PostCreate = std::move(other.PostCreate);

		other.hndl = nullptr;
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

#ifdef _DEBUG
	/* We do an additional check on debug mode to see if non of the attachment indices overlap. */
	std::set<uint32> references;
	uint32 i = 0;
	for (const Subpass &subpass : subpasses)
	{
		for (const Output &output : subpass.outputs)
		{
			if (references.find(output.reference.Attachment) != references.end())
			{
				Log::Fatal("Output '%s' in subpass %u uses an already defined attachment index (%u)!",
					output.GetInfo().Name.c_str(), i, output.reference.Attachment);
			}

			references.emplace(output.reference.Attachment);
		}

		++i;
	}
#endif

	CreateRenderpass();

	PostCreate.Post(*this);
	MarkAsLoaded(viaLoader, L"Renderpass");
}

void Pu::Renderpass::CreateRenderpass(void)
{
	/* We aren't going to use the old clear values anymore. */
	clearValues.clear();

	/* 
	Pre-set the size of the attachment descriptions. 
	The order of the descriptions is based on the references set by the user (or kept as default).
	*/
	size_t reserveSize = 0;
	vector<AttachmentDescription> attachmentDescriptions;
	for (const Subpass &subpass : subpasses) reserveSize += subpass.outputs.size();
	attachmentDescriptions.resize(reserveSize);
	clearValues.resize(reserveSize);

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

			/* Set the correct attachment description and clear value. */
			attachmentDescriptions[output.reference.Attachment] = output.description;
			clearValues[output.reference.Attachment] = output.clear;
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
					for (AttachmentReference ref : attachments)
					{
						/* The input attachment will have the same index as a previous subpass color attachment. */
						if (ref.Attachment == idx)
						{
							/* The layout of input attachments can only be read only or general, general is bad. */
							ref.Layout = ImageLayout::ShaderReadOnlyOptimal;
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

void Pu::Renderpass::Destroy(void)
{
	if (hndl) device->vkDestroyRenderPass(device->hndl, hndl, nullptr);

	/* This means we loaded the shaders inline, which means we need to free them. */
	if (ownsShaders)
	{
		for (const Subpass &subpass : subpasses)
		{
			for (const Shader *shader : subpass.GetShaders()) delete shader;
		}
	}
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
	for (Subpass &subpass : renderpass.subpasses) subpass.Link(*renderpass.device);

	/* Delete the underlying shader load tasks, create the renderpass and finally allow the scheduler to delete this task. */
	for (const Shader::LoadTask *task : children) delete task;
	renderpass.Create(true);
	return Result::AutoDelete();
}