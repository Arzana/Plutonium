#include "Graphics/Vulkan/Shaders/ShaderProgram.h"
#include "Content/AssetCache.h"

using namespace Pu;

FieldInfo ShaderProgram::defInfo = FieldInfo(0, "", FieldType(ComponentType::Invalid, SizeType::Scalar), spv::StorageClass::Generic, Decoration());
Output ShaderProgram::defOutput = Output(ShaderProgram::defInfo, 0, OutputUsage::Unknown);
Attribute ShaderProgram::defAttrib = Attribute(defInfo);
Descriptor ShaderProgram::defDescr = Descriptor(defInfo);
PushConstant ShaderProgram::defConst = PushConstant(defInfo);

Pu::ShaderProgram::ShaderProgram(void)
	: Asset(true), linkSuccessfull(false), PostLink("ShaderProgramPostLink")
{}

Pu::ShaderProgram::ShaderProgram(LogicalDevice & device, const vector<Shader*>& shaderModules)
	: Asset(true), linkSuccessfull(false), shaders(shaderModules), PostLink("ShaderProgramPostLink")
{
	SetHash(std::hash<wstring>{}(shaderModules.select<wstring>([](const Shader *shader) { return shader->GetName(); })));
	Link(device, false);
}

Output & Pu::ShaderProgram::GetOutput(const string & name)
{
	for (Output &cur : outputs)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find output field '%s'!", name.c_str());
	return defOutput;
}

const Output & Pu::ShaderProgram::GetOutput(const string & name) const
{
	for (const Output &cur : outputs)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find output field '%s'!", name.c_str());
	return defOutput;
}

Attribute & Pu::ShaderProgram::GetAttribute(const string & name)
{
	for (Attribute &cur : attributes)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find attribute field '%s'!", name.c_str());
	return defAttrib;
}

const Attribute & Pu::ShaderProgram::GetAttribute(const string & name) const
{
	for (const Attribute &cur : attributes)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find attribute field '%s'!", name.c_str());
	return defAttrib;
}

Descriptor & Pu::ShaderProgram::GetDescriptor(const string & name)
{
	for (Descriptor &cur : descriptors)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find descriptor field '%s'!", name.c_str());
	return defDescr;
}

const Descriptor & Pu::ShaderProgram::GetDescriptor(const string & name) const
{
	for (const Descriptor &cur : descriptors)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find descriptor field '%s'!", name.c_str());
	return defDescr;
}

PushConstant & Pu::ShaderProgram::GetPushConstant(const string & name)
{
	for (PushConstant &cur : pushConstants)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find push constant field '%s'!", name.c_str());
	return defConst;
}

const PushConstant & Pu::ShaderProgram::GetPushConstant(const string & name) const
{
	for (const PushConstant &cur : pushConstants)
	{
		if (name == cur.GetInfo().Name) return cur;
	}

	Log::Error("Unable to find push constant field '%s'!", name.c_str());
	return defConst;
}

Asset & Pu::ShaderProgram::Duplicate(AssetCache & cache)
{
	/* The underlying shaders are assets, so reference them. */
	for (Shader *shader : shaders) shader->Reference();

	/* Create a new shader program and override the hash and give it a new instance hash. */
	ShaderProgram *result = new ShaderProgram(shaders.front()->GetDevice(), shaders);
	result->SetHash(GetHash(), cache.RngHash(GetHash()));
	cache.Store(result);

	return *result;
}

void Pu::ShaderProgram::Link(LogicalDevice & device, bool viaLoader)
{
#ifdef _DEBUG
	for (size_t i = 0; i < shaders.size(); i++)
	{
		/* Make sure compute shaders aren't thrown in the mix. */
		if (shaders.size() > 1 && shaders[i]->GetType() == ShaderStageFlags::Compute)
		{
			Log::Error("Compute shaders can only apear as seperate entities in a shader program!");
			PostLink.Post(*this);
			return;
		}

		/* Check if no shader modules where added with the same type. */
		for (size_t j = 0; j < shaders.size(); j++)
		{
			if (i != j && shaders[i]->GetType() == shaders[j]->GetType())
			{
				Log::Error("Cannot add two %s shaders in one shader prorgam!", to_string(shaders[i]->GetType()));
				PostLink.Post(*this);
				return;
			}
		}
	}
#endif

	/* Sort all shaders on their invokation time in the Vulkan pipeline (Vertex -> Tessellation -> Geometry -> Fragment). */
	std::sort(shaders.begin(), shaders.end(), [](const Shader *a, const Shader *b)
		{
			return _CrtEnum2Int(a->GetType()) < _CrtEnum2Int(b->GetType());
		});

#ifdef _DEBUG
	/* Only perform link checking for non-compute shader programs. */
	if (shaders.size() != 1 && shaders[0]->GetType() != ShaderStageFlags::Compute)
	{
		/* Check if the supplied shader modules can be linked together. */
		for (size_t i = 0, j = 1; j < shaders.size(); i++, j++)
		{
			if (!CheckIO(*shaders[i], *shaders[j]))
			{
				/* Shader is done loading but failed linking because of I/O issues. */
				Log::Error("Unable to link %s shader to %s shader!", to_string(shaders[i]->GetType()), to_string(shaders[j]->GetType()));
				PostLink.Post(*this);
				return;
			}
		}
	}
#endif

	/* We need to fields to be loaded in order to check the descriptors. */
	LoadFields(device.GetPhysicalDevice());

#ifdef _DEBUG
	if (!CheckSets())
	{
		/* Shader is done loading but failed linking because of descriptor set overlap. */
		Log::Error("Unable to link %zu shaders!", shaders.size());
		PostLink.Post(*this);
		return;
	}
#endif

	/* At this point we know that the linking was successfull so we can start creating set layouts. */
	linkSuccessfull = true;
	CreateSetLayouts(device);

#ifdef _DEBUG
	/* Log the creation of the shader progam on debug mode. */
	wstring modules;
	for (size_t i = 0; i < shaders.size(); i++)
	{
		modules += shaders[i]->GetName();
		if (i + 1 < shaders.size()) modules += L" -> ";
	}

	Log::Verbose("Successfully created shader program %ls.", modules.c_str());
#endif

	PostLink.Post(*this);
	MarkAsLoaded(viaLoader, L"ShaderProgram");
}

void Pu::ShaderProgram::LoadFields(const PhysicalDevice & physicalDevice)
{
	/* Load all the input attributes (these are the ones we have to set se they'll always be in the first shader). */
	for (const FieldInfo &info : shaders.front()->fields)
	{
		if (info.Storage == spv::StorageClass::Input) attributes.emplace_back(Attribute(info));
	}

	/*
	Load all the outputs, these have to be color attachments because we cannot check for depth/stencil via reflection,
	preserve makes no sense to save here, input also cannot be checked for and resolve is not curentlt handled.
	Outputs can only be defined in the last shader module of the shader program.
	*/
	for (const FieldInfo &info : shaders.back()->fields)
	{
		if (info.Storage == spv::StorageClass::Output) outputs.emplace_back(Output(info, static_cast<uint32>(outputs.size()), OutputUsage::Color));
	}

	/* Load all the descriptors from any shader. */
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

bool Pu::ShaderProgram::CheckIO(const Shader & a, const Shader & b) const
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

bool Pu::ShaderProgram::CheckSets(void) const
{
	const std::map<uint32, vector<const Descriptor*>> sets = QuerySets();
	bool result = true;

	for (const auto &[set, arguments] : sets)
	{
		for (const Descriptor *first : arguments)
		{
			for (const Descriptor *second : arguments)
			{
				if (first == second) continue;

				/* Descriptor with the same binding might be part of a uniform buffer or them might be invalid. */
				if (first->GetBinding() == second->GetBinding())
				{
					/* The types should always be equal (i.e. UniformBuffer vs Image). */
					if (first->GetType() != second->GetType())
					{
						Log::Error("Binding %u in set %u is both used for %s %s and %s %s", first->GetBinding(), set,
							to_string(first->GetType()), first->GetInfo().Name.c_str(),
							to_string(second->GetType()), second->GetInfo().Name.c_str());

						result = false;
					}
					else if (first->GetInfo().Name != second->GetInfo().Name)
					{
						/* Only UniformBuffers can share a binding when the name is not the same. */
						if (first->GetType() != DescriptorType::UniformBuffer)
						{
							Log::Error("Binding %u in set %u is used for both %s and %s!", first->GetBinding(), set, first->GetInfo().Name.c_str(), second->GetInfo().Name.c_str());
							result = false;
						}
					}
				}
			}
		}
	}

	return result;
}

std::map<uint32, vector<const Descriptor*>> Pu::ShaderProgram::QuerySets(void) const
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

	return sets;
}

void Pu::ShaderProgram::CreateSetLayouts(LogicalDevice & device)
{
	const std::map<uint32, vector<const Descriptor*>> sets = QuerySets();

	/* Allocate the descriptor set layouts. */
	setLayouts.reserve(sets.size());
	for (const auto &[set, arguments] : sets)
	{
		setLayouts.emplace_back(device, arguments);
	}
}