#include "Graphics/Vulkan/Shaders/Subpass.h"

using namespace Pu;

FieldInfo Subpass::dsInfo = FieldInfo(0, "DepthStencil", FieldType(ComponentType::Float, SizeType::Scalar), spv::StorageClass::Output, Decoration());
FieldInfo Subpass::defInfo = FieldInfo(0, "", FieldType(ComponentType::Invalid, SizeType::Scalar), spv::StorageClass::Generic, Decoration());
Output Subpass::defOutput = Output(Subpass::defInfo, 0, OutputUsage::Unknown);
Attribute Subpass::defAttrib = Attribute(defInfo);
Descriptor Subpass::defDescr = Descriptor(defInfo);
PushConstant Subpass::defConst = PushConstant(defInfo);

Pu::Subpass::Subpass()
	: linkSuccessfull(false), ds(nullptr), dependencyUsed(false)
{}

Subpass::Subpass(LogicalDevice & device, std::initializer_list<Shader*> shaderModules)
	: linkSuccessfull(false), ds(nullptr), shaders(shaderModules), dependencyUsed(false)
{
	Link(device);
}

Pu::Subpass::Subpass(LogicalDevice & device, const vector<Shader*>& shaderModules)
	: linkSuccessfull(false), ds(nullptr), shaders(shaderModules), dependencyUsed(false)
{
	Link(device);
}

void Pu::Subpass::SetDependency(PipelineStageFlag srcStage, PipelineStageFlag dstStage, AccessFlag srcAccess, AccessFlag dstAccess, DependencyFlag flags)
{
	dependencyUsed = true;
	dependency.SrcStageMask = srcStage;
	dependency.SrcAccessMask = srcAccess;
	dependency.DstStageMask = dstStage;
	dependency.DstAccessMask = dstAccess;
	dependency.DependencyFlags = flags;
}

Output & Subpass::AddDepthStencil(void)
{
	/* Make sure we don't attach multiple depth/stencil attachment. */
	if (ds)
	{
		Log::Warning("A depth/stencil attachment has already added to the subpass, returning old one!");
		return *ds;
	}

	/* Create a dummy output and add it to our list. */
	outputs.emplace_back(Output(dsInfo, static_cast<uint32>(outputs.size()), OutputUsage::DepthStencil));
	ds = &outputs.back();
	return outputs.back();
}

void Pu::Subpass::CloneDepthStencil(uint32 referenceIndex)
{
	Output &output = AddDepthStencil();
	output.SetReference(referenceIndex);
	output.clone = true;
}

Output & Pu::Subpass::GetOutput(const string & name)
{
	for (Output &cur : outputs)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find output field '%s'!", name.c_str());
	return defOutput;
}

const Output & Pu::Subpass::GetOutput(const string & name) const
{
	for (const Output &cur : outputs)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find output field '%s'!", name.c_str());
	return defOutput;
}

void Pu::Subpass::CloneOutput(const string & name, uint32 referenceIndex)
{
	Output &output = GetOutput(name);
	output.SetReference(referenceIndex);
	output.clone = true;
}

Attribute & Pu::Subpass::GetAttribute(const string & name)
{
	for (Attribute &cur : attributes)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find attribute field '%s'!", name.c_str());
	return defAttrib;
}

const Attribute & Pu::Subpass::GetAttribute(const string & name) const
{
	for (const Attribute &cur : attributes)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find attribute field '%s'!", name.c_str());
	return defAttrib;
}

Descriptor & Pu::Subpass::GetDescriptor(const string & name)
{
	for (Descriptor &cur : descriptors)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find descriptor field '%s'!", name.c_str());
	return defDescr;
}

const Descriptor & Pu::Subpass::GetDescriptor(const string & name) const
{
	for (const Descriptor &cur : descriptors)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find descriptor field '%s'!", name.c_str());
	return defDescr;
}

PushConstant & Pu::Subpass::GetPushConstant(const string & name)
{
	for (PushConstant &cur : pushConstants)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find push constant field '%s'!", name.c_str());
	return defConst;
}

const PushConstant & Pu::Subpass::GetPushConstant(const string & name) const
{
	for (const PushConstant &cur : pushConstants)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find push constant field '%s'!", name.c_str());
	return defConst;
}

void Pu::Subpass::Link(LogicalDevice & device)
{
#ifdef _DEBUG
	for (size_t i = 0; i < shaders.size(); i++)
	{
		/* Make sure compute shaders aren't thrown in the mix. */
		if (shaders.size() > 1 && shaders[i]->GetType() == ShaderStageFlag::Compute)
		{
			Log::Error("Compute shaders can only apear as seperate entities in a subpass!");
			return;
		}

		/* Check if no shader modules where added with the same type. */
		for (size_t j = 0; j < shaders.size(); j++)
		{
			if (i != j && shaders[i]->GetType() == shaders[j]->GetType())
			{
				Log::Error("Cannot add two %s shaders in one subpass!", to_string(shaders[i]->GetType()));
				return;
			}
		}
	}
#endif

	/* Sort all subpasses on their invokation time in the Vulkan pipeline (Vertex -> Tessellation -> Geometry -> Fragment). */
	std::sort(shaders.begin(), shaders.end(), [](const Shader *a, const Shader *b)
	{
		return _CrtEnum2Int(a->GetType()) < _CrtEnum2Int(b->GetType());
	});

#ifdef _DEBUG
	/* Check if the supplied shader modules can be linked together. */
	for (size_t i = 0, j = 1; j < shaders.size(); i++, j++)
	{
		if (!CheckIO(*shaders[i], *shaders[j]))
		{
			/* Shader is done loading but failed linking. */
			Log::Error("Unable to link %s shader to %s shader!", to_string(shaders[i]->GetType()), to_string(shaders[j]->GetType()));
			return;
		}
	}
#endif

	/* At this point we know that the linking was successfull so we can start loading the fields. */
	linkSuccessfull = true;
	LoadFields(device.GetPhysicalDevice());
	CreateSetLayouts(device);

	/* Log the creation of the subpass on debug mode. */
#ifdef _DEBUG
	wstring modules;
	for (const Shader *shader : shaders)
	{
		modules += shader->GetName();
		if (shader->GetType() != ShaderStageFlag::Fragment) modules += L" -> ";
	}

	Log::Verbose("Successfully created subpass %ls.", modules.c_str());
#endif
}

void Pu::Subpass::LoadFields(const PhysicalDevice & physicalDevice)
{
	/* Load all the input attributes (these are the ones we have to set se they'll always be in the first shader). */
	for (const FieldInfo &info : shaders.front()->fields)
	{
		if (info.Storage == spv::StorageClass::Input) attributes.emplace_back(Attribute(info));
	}

	/*
	Load all the outputs, these have to be color attachments because we cannot check for depth/stencil via reflection,
	preserve makes no sense to save here, input also cannot be checked for and resolve is not curentlt handled.
	Outputs can only be defined in the last shader module of the subpass.
	*/
	for (const FieldInfo &info : shaders.back()->fields)
	{
		if (info.Storage == spv::StorageClass::Output) outputs.emplace_back(Output(info, static_cast<uint32>(outputs.size()), OutputUsage::Color));
	}

	/* Load all the descriptors from any subpass. */
	for (const Shader *shader : shaders)
	{
		for (const FieldInfo &info : shader->fields)
		{
			if (info.Storage == spv::StorageClass::UniformConstant || info.Storage == spv::StorageClass::Uniform)
			{
				descriptors.emplace_back(Descriptor(physicalDevice, info, shader->GetType()));
			}
			else if (info.Storage == spv::StorageClass::PushConstant)
			{
				pushConstants.emplace_back(PushConstant(physicalDevice, info, shader->GetType()));
			}
		}
	}
}

bool Pu::Subpass::CheckIO(const Shader & a, const Shader & b)
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
					Log::Error("The type of '%s' (%ls) does't match '%s' (%ls)!", aInfo.Name.c_str(), a.GetName().c_str(), bInfo.Name.c_str(), b.GetName().c_str());
					result = false;
					continue;
				}

				/* Check if the input is linked to multiple outputs. */
				if (linkedLocations.contains(bInfo.GetLocation()))
				{
					Log::Error("Multiple output fields are using '%s' (%ls) as an input field!", bInfo.Name.c_str(), b.GetName().c_str());
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
			Log::Error("Unable top find matching field in %s shader (%ls) for %s in %s shader (%ls)!", to_string(b.GetType()), b.GetName().c_str(), aInfo.Name.c_str(), to_string(a.GetType()), b.GetName().c_str());
			result = false;
		}
	}

	/* Check if any input fields of b have gone unset. */
	for (const FieldInfo &info : b.fields)
	{
		if (info.Storage != spv::StorageClass::Input) continue;

		if (!linkedLocations.contains(info.GetLocation()))
		{
			Log::Error("%s in %s shader (%ls) was not set by previous %s shader (%ls)!", info.Name.c_str(), to_string(b.GetType()), b.GetName().c_str(), to_string(a.GetType()), b.GetName().c_str());
			result = false;
		}
	}

	return result;
}

void Pu::Subpass::CreateSetLayouts(LogicalDevice & device)
{
	/* Get all the sets in this subpass and their descriptors. */
	std::map<uint32, vector<const Descriptor*>> sets;
	for (const Descriptor &descriptor : descriptors)
	{
		decltype(sets)::iterator it = sets.find(descriptor.set);
		if (it != sets.end()) it->second.emplace_back(&descriptor);
		else
		{
			vector<const Descriptor*> buffer{ { &descriptor } };
			sets.emplace(descriptor.set, std::move(buffer));
		}
	}

	/* Allocate the descriptor set layouts. */
	setLayouts.reserve(sets.size());
	for (const auto &[set, arguments] : sets)
	{
		setLayouts.emplace_back(device, arguments);
	}
}