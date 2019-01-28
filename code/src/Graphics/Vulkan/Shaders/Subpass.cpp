#include "Graphics/Vulkan/Shaders/Subpass.h"
#include "Streams/FileUtils.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"

const Pu::FieldInfo Pu::Subpass::invalid = Pu::FieldInfo();

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
	name = path;

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
		HandleVariable(id, typeId, storage);
	}

	/* Clear the temporary buffers. */
	names.clear();
	memberNames.clear();
	decorations.clear();
	typedefs.clear();
	types.clear();
	structs.clear();
	variables.clear();
}

void Pu::Subpass::HandleVariable(spv::Id id, spv::Id typeId, spv::StorageClass storage)
{
	const spv::Id typePointer = typedefs[typeId];

	/* Handle normal type. */
	if (types.find(typePointer) != types.end())
	{
		/* User defined variables need to have at least one handlable decoration, all others must be build in variables, which we can skip. */
		if (decorations.find(id) != decorations.end())
		{
			/* Only handle types with defined names, this should never occur. */
			fields.emplace_back(id, std::move(names[id]), types[typePointer], storage, decorations[id]);
		}
	}
	/* Handle struct types. */
	else if (structs.find(typePointer) != structs.end())
	{
		/* Handle all member types. */
		const vector<spv::Id> &members = structs[typePointer];
		for (size_t i = 0, offset = 0; i < members.size(); i++)
		{
			const spv::Id memberTypeId = members[i];
			const FieldTypes type = types[memberTypeId];
			string &memberName = memberNames[typePointer][i];

			/* Skip any build in members (defined with 'gl_' prefix). */
			if (!memberName.contains("gl_"))
			{
				Decoration memberDecoration(offset);
				memberDecoration.Merge(decorations[id]);

				fields.emplace_back(memberTypeId, std::move(memberName), type, storage, memberDecoration);
			}

			/* Increase the offset. */
			offset += sizeof_fieldType(type);
		}
	}
}

void Pu::Subpass::HandleModule(SPIRVReader & reader, spv::Op opCode, size_t wordCnt)
{
	/* Pass usefull operation codes to their functions. */
	switch (opCode)
	{
	case (spv::Op::OpName):
		HandleName(reader);
		break;
	case (spv::Op::OpMemberName):
		HandleMemberName(reader);
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
	case (spv::Op::OpTypeStruct):
		HandleStruct(reader, wordCnt - 1);
		break;
		//TODO: handle array types.
	case (spv::Op::OpTypeImage):
		HandleImage(reader);
		break;
	case (spv::Op::OpTypeSampledImage):
		HandleSampledImage(reader);
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

void Pu::Subpass::HandleMemberName(SPIRVReader & reader)
{
	const spv::Id type = reader.ReadWord();
	const spv::Id idx = reader.ReadWord();
	const string str = reader.ReadLiteralString();

	if (memberNames.find(type) != memberNames.end())
	{
		/* Make sure we have enough space. */
		memberNames[type].resize(idx + 1);
		memberNames[type][idx] = str;
	}
	else
	{
		/* If we cannot find the containing struct yet, just add it. */
		vector<string> members = { str };
		memberNames.emplace(type, members);
	}
}

void Pu::Subpass::HandleDecorate(SPIRVReader & reader)
{
	/* Read the target and create the decoration object. */
	const spv::Id target = reader.ReadWord();
	const spv::Decoration type = _CrtInt2Enum<spv::Decoration>(reader.ReadWord());

	Decoration result;
	switch (type)
	{
	case (spv::Decoration::Location):		// Location decoration stores a single literal number.
		result.Numbers.emplace(type, reader.ReadWord());
		break;
	case (spv::Decoration::DescriptorSet):	// Descriptor set decoration stores a single literal number.
		result.Numbers.emplace(type, reader.ReadWord());
		break;
	case (spv::Decoration::Binding):		// Binding decoration stores a single literal number.
		result.Numbers.emplace(type, reader.ReadWord());
		break;
	default:								// Just log that we don't hanle the decoration at this point.
		Log::Warning("Unable to handle decoration type '%s' at this point!", to_string(type));
		return;
	}

	/* Only push if the decoration type was handled, also merge them together if the target is already added. */
	if (decorations.find(target) != decorations.end()) decorations[target].Merge(result);
	else decorations.emplace(target, result);
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

void Pu::Subpass::HandleStruct(SPIRVReader & reader, size_t memberCnt)
{
	const spv::Id id = reader.ReadWord();
	vector<spv::Id> members;
	members.reserve(memberCnt);
	for (size_t i = 0; i < memberCnt; i++) members.emplace_back(reader.ReadWord());

	structs.emplace(id, std::move(members));
}

void Pu::Subpass::HandleImage(SPIRVReader & reader)
{
	/* We need the type id but we can skip the underlying image type as it doesn't concern us. */
	const spv::Id id = reader.ReadWord();
	reader.AdvanceWord();

	FieldTypes type = FieldTypes::Invalid;
	switch (_CrtInt2Enum<spv::Dim>(reader.ReadWord()))
	{
	case (spv::Dim::Dim1D):
		type = FieldTypes::Image1D;
		break;
	case (spv::Dim::Dim2D):
		type = FieldTypes::Image2D;
		break;
	case (spv::Dim::Dim3D):
		type = FieldTypes::Image3D;
		break;
	case (spv::Dim::Cube):
		type = FieldTypes::ImageCube;
		break;
	}

	/*
	We can also skip the:
	depth indicator,
	arrayed indicator,
	multi-sample indicator,
	sampled indicator
	format
	and (optional) access qualifier Because we don't care about them.
	*/

	types.emplace(id, type);
}

void Pu::Subpass::HandleSampledImage(SPIRVReader & reader)
{
	/* Just add the type of the image that's being sampled to the type list. */
	const spv::Id id = reader.ReadWord();
	const FieldTypes imgType = types[reader.ReadWord()];
	types.emplace(id, imgType);
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