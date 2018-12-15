#include "Graphics/Vulkan/Shaders/Subpass.h"
#include "Streams/FileUtils.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"

const Pu::FieldInfo Pu::Subpass::invalid = Pu::FieldInfo();
constexpr const char *KEY_LOCATION = "Location";

Pu::Subpass::Subpass(LogicalDevice & device)
	: parent(device), loaded(false)
{}

Pu::Subpass::Subpass(LogicalDevice & device, const string & path)
	: parent(device)
{
	Load(path);
}

Pu::Subpass::Subpass(Subpass && value)
	: parent(value.parent), info(value.info), fields(std::move(value.fields)), loaded(value.IsLoaded())
{
	value.info.Module = nullptr;
}

Pu::Subpass & Pu::Subpass::operator=(Subpass && other)
{
	if (this != &other)
	{
		Destroy();
		parent = std::move(other.parent);
		info = other.info;
		fields = std::move(other.fields);
		loaded.store(other.IsLoaded());

		other.info.Module = nullptr;
		other.loaded.store(false);
	}

	return *this;
}

/* Name hides class member, checked and caused no unexpected behaviour. */
#pragma warning(push)
#pragma warning(disable:4458)
const Pu::FieldInfo & Pu::Subpass::GetField(const string & name) const
{
	for (const FieldInfo &cur : fields)
	{
		if (name == cur.Name) return cur;
	}

	return invalid;
}
#pragma warning(pop)

void Pu::Subpass::Load(const string & path)
{
	const string ext = _CrtGetFileExtension(path);
	name = _CrtGetFileNameWithoutExtension(path);

	if (ext == "spv")
	{
		/* If the input shader is already defined as binary just load it. */
		Create(path);
		SetInfo(_CrtGetFileExtension(string(path, path.length() - 4)));
	}
	else
	{
		/* First compile the shader to SPIR-V and then load it. */
		Create(SPIRV::FromGLSLPath(path));
		SetInfo(ext);
	}

	/* Set the information of the subpass. */
	SetFieldInfo();
	loaded.store(true);
}

void Pu::Subpass::Create(const string & path)
{
	/* Make sure the file exists before trying to load it. */
	if (FileReader::FileExists(path.c_str()))
	{
		/* Load the SPIR-V file with a specialized reader. */
		SPIRVReader spvr(path.c_str());

		/* Compile the SPIR-V shader module. */
		ShaderModuleCreateInfo createInfo(spvr.GetStream().GetSize(), spvr.GetStream().GetData());
		VK_VALIDATE(parent.vkCreateShaderModule(parent.hndl, &createInfo, nullptr, &info.Module), PFN_vkCreateShaderModule);

		/* Perform reflection to get the inputs and outputs. */
		spvr.HandleAllModules(SPIRVReader::ModuleHandler(*this, &Subpass::HandleModule));
	}
	else Log::Fatal("Unable to load shader module!");
}

void Pu::Subpass::SetFieldInfo(void)
{
	/* Create field information for all fields. */
	for (const auto&[id, typeId, storage] : variables)
	{
		/* Skip variables that have no name or have a none supported type. */
		if (ShouldHandleField(id, typeId))
		{
			fields.emplace_back(id, std::move(names[id]), types[typedefs[typeId]], storage, decorations[id].Numbers[KEY_LOCATION]);
		}
	}

	/* Clear the temporary buffers. */
	names.clear();
	decorations.clear();
	typedefs.clear();
	types.clear();
	variables.clear();
}

bool Pu::Subpass::ShouldHandleField(spv::Id id, spv::Id typeId)
{
	/* Variables without a name are sometimes defined by SPIR-V but we have no use for them. */
	if (names[id].empty()) return false;

	/* We can only handle a subset of the types, so ignore the ones that we didn't load. */
	if (types.find(typedefs[typeId]) == types.end()) return false;

	/* User defined variables need to have a location, all others must be build in variables, which we can skip. */
	if (decorations.find(id) == decorations.end()) return false;
	if (decorations[id].Numbers.find(KEY_LOCATION) == decorations[id].Numbers.end()) return false;

	return true;
}

void Pu::Subpass::HandleModule(SPIRVReader & reader, spv::Op opCode, size_t)
{
	/* Pass usefull operation codes to their functions. */
	switch (opCode)
	{
	case (spv::Op::OpName):
		HandleName(reader);
		break;
	case (spv::Op::OpDecorate):
		HandleDecorate(reader);
		break;
	case (spv::Op::OpTypePointer):
		HandleType(reader);
		break;
	case (spv::Op::OpTypeInt):
		HandleInt(reader);
		break;
	case (spv::Op::OpTypeFloat):
		HandleFloat(reader);
		break;
	case (spv::Op::OpTypeVector):
		HandleVector(reader);
		break;
	case (spv::Op::OpTypeMatrix):
		HandleMatrix(reader);
		break;
	case (spv::Op::OpVariable):
		HandleVariable(reader);
		break;
	}
}

void Pu::Subpass::HandleName(SPIRVReader & reader)
{
	const spv::Id target = reader.ReadWord();
	const string str = reader.ReadLiteralString();
	names.emplace(target, str);
}

void Pu::Subpass::HandleDecorate(SPIRVReader & reader)
{
	/* Read the target and create the decoration object. */
	const spv::Id target = reader.ReadWord();
	Decoration result(_CrtInt2Enum<spv::Decoration>(reader.ReadWord()));

	switch (result.Type)
	{
	case (spv::Decoration::Location):	// Location decoration stores a single literal number.
		result.Numbers.emplace(KEY_LOCATION, reader.ReadWord());
		break;
	default:							// Just log that we don't hanle the decoration at this point.
		Log::Warning("Unable to handle decoration type '%s' at this point!", to_string(result.Type));
		return;
	}

	/* Only push if the decoration type was handled. */
	decorations.emplace(target, result);
}

void Pu::Subpass::HandleType(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	reader.AdvanceWord();	// storage class.
	const spv::Id type = reader.ReadWord();
	typedefs.emplace(id, type);
}

void Pu::Subpass::HandleInt(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const spv::Word width = reader.ReadWord();
	const bool isSigned = reader.ReadWord();

	FieldTypes type = FieldTypes::Invalid;
	if (width == 8) type = isSigned ? FieldTypes::SByte : FieldTypes::Byte;
	else if (width == 16) type = isSigned ? FieldTypes::Short : FieldTypes::UShort;
	else if (width == 32) type = isSigned ? FieldTypes::Int : FieldTypes::UInt;
	else if (width == 64) type = isSigned ? FieldTypes::Long : FieldTypes::ULong;

	types.emplace(id, type);
}

void Pu::Subpass::HandleFloat(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const spv::Word width = reader.ReadWord();

	FieldTypes type = FieldTypes::Invalid;
	if (width == 16) type = FieldTypes::HalfFloat;
	else if (width == 32) type = FieldTypes::Float;
	else if (width == 64) type = FieldTypes::Double;

	types.emplace(id, type);
}

void Pu::Subpass::HandleVector(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const FieldTypes componentType = types[reader.ReadWord()];
	const spv::Word componentCnt = reader.ReadWord();

	FieldTypes type = FieldTypes::Invalid;
	if (componentType == FieldTypes::Float)
	{
		if (componentCnt == 2) type = FieldTypes::Vec2;
		else if (componentCnt == 3) type = FieldTypes::Vec3;
		else if (componentCnt == 4) type = FieldTypes::Vec4;
	}

	types.emplace(id, type);
}

void Pu::Subpass::HandleMatrix(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const FieldTypes columnType = types[reader.ReadWord()];
	const spv::Word columnCnt = reader.ReadWord();

	FieldTypes type = FieldTypes::Invalid;
	if (columnType == FieldTypes::Vec4 && columnCnt == 4) type = FieldTypes::Matrix;

	types.emplace(id, type);
}

void Pu::Subpass::HandleVariable(SPIRVReader & reader)
{
	const spv::Id resultType = reader.ReadWord();
	const spv::Id resultId = reader.ReadWord();
	const spv::StorageClass storageClass = _CrtInt2Enum<spv::StorageClass>(reader.ReadWord());
	
	variables.emplace_back(std::make_tuple(resultId, resultType, storageClass));
}

void Pu::Subpass::SetInfo(const string & ext)
{
	if (ext == "vert") info.Stage = ShaderStageFlag::Vertex;
	else if (ext == "tesc") info.Stage = ShaderStageFlag::TessellationControl;
	else if (ext == "tese") info.Stage = ShaderStageFlag::TessellationEvaluation;
	else if (ext == "geom") info.Stage = ShaderStageFlag::Geometry;
	else if (ext == "frag") info.Stage = ShaderStageFlag::Fragment;
	else if (ext == "comp") info.Stage = ShaderStageFlag::Compute;
}

void Pu::Subpass::Destroy(void)
{
	if (info.Module) parent.vkDestroyShaderModule(parent.hndl, info.Module, nullptr);
}

Pu::Subpass::LoadTask::LoadTask(Subpass & result, const string & path)
	: result(result), path(path)
{}

Pu::Task::Result Pu::Subpass::LoadTask::Execute(void)
{
	result.Load(path);
	return Result();
}