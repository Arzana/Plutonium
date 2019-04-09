#include "Graphics/Vulkan/Shaders/Shader.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"

const Pu::FieldInfo Pu::Shader::invalid = Pu::FieldInfo();

Pu::Shader::Shader(LogicalDevice & device)
	: Asset(true), parent(device)
{}

Pu::Shader::Shader(LogicalDevice & device, const wstring & path)
	: Asset(true, std::hash<wstring>{}(path)), parent(device)
{
	Load(path, false);
}

Pu::Shader::Shader(Shader && value)
	: Asset(std::move(value)), parent(value.parent), info(value.info), fields(std::move(value.fields))
{
	value.info.Module = nullptr;
}

Pu::Shader & Pu::Shader::operator=(Shader && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		parent = std::move(other.parent);
		info = other.info;
		fields = std::move(other.fields);

		other.info.Module = nullptr;
	}

	return *this;
}

/* Name hides class member, checked and caused no unexpected behaviour. */
#pragma warning(push)
#pragma warning(disable:4458)
const Pu::FieldInfo & Pu::Shader::GetField(const string & name) const
{
	for (const FieldInfo &cur : fields)
	{
		if (name == cur.Name) return cur;
	}

	return invalid;
}
#pragma warning(pop)

Pu::Asset & Pu::Shader::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

void Pu::Shader::Load(const wstring & path, bool viaLoader)
{
	const wstring ext = path.fileExtension().toUpper();
	name = path;

	if (ext == L"SPV")
	{
		/* If the input shader is already defined as binary just load it. */
		Create(path);
		SetInfo(wstring(path, path.length() - 4).fileExtension().toUpper());
	}
	else
	{
		/* First compile the shader to SPIR-V and then load it. */
		Create(SPIRV::FromGLSLPath(path));
		SetInfo(ext);
	}

	/* Set the information of the subpass. */
	SetFieldInfo();
	MarkAsLoaded(viaLoader);
}

void Pu::Shader::Create(const wstring & path)
{
	/* Make sure the file exists before trying to load it. */
	if (FileReader::FileExists(path))
	{
		/* Load the SPIR-V file with a specialized reader. */
		SPIRVReader spvr(path);

		/* Compile the SPIR-V shader module. */
		ShaderModuleCreateInfo createInfo(spvr.GetStream().GetSize(), spvr.GetStream().GetData());
		VK_VALIDATE(parent.vkCreateShaderModule(parent.hndl, &createInfo, nullptr, &info.Module), PFN_vkCreateShaderModule);

		/* Perform reflection to get the inputs and outputs. */
		auto handler = DelegateMethod<SPIRVReader, Shader, spv::Op, size_t>(*this, &Shader::HandleModule);
		spvr.HandleAllModules(handler);
	}
	else Log::Fatal("Unable to load shader module!");
}

void Pu::Shader::SetFieldInfo(void)
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

void Pu::Shader::HandleVariable(spv::Id id, spv::Id typeId, spv::StorageClass storage)
{
	const spv::Id typePointer = typedefs[typeId];

	/* Handle normal type. */
	if (types.find(typePointer) != types.end())
	{
		/* User defined variables need to have at least one handlable decoration, all others must be build in variables, which we can skip. */
		if (decorations.find(id) != decorations.end())
		{
			/* Only handle types with defined names, this should never occur. */
			fields.emplace_back(id, std::move(names[id]), std::move(types[typePointer]), storage, decorations[id]);
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
			FieldType fieldType = types[memberTypeId];
			string &memberName = memberNames[typePointer][i];

			/* Skip any build in members (defined with 'gl_' prefix). */
			if (!memberName.contains("gl_"))
			{
				Decoration memberDecoration(offset);
				memberDecoration.Merge(decorations[id]);

				fields.emplace_back(memberTypeId, std::move(memberName), std::move(fieldType), storage, memberDecoration);
			}

			/* Increase the offset. */
			offset += fieldType.GetSize();
		}
	}
}

void Pu::Shader::HandleModule(SPIRVReader & reader, spv::Op opCode, size_t wordCnt)
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

void Pu::Shader::HandleName(SPIRVReader & reader)
{
	const spv::Id target = reader.ReadWord();
	const string str = reader.ReadLiteralString();
	names.emplace(target, str);
}

void Pu::Shader::HandleMemberName(SPIRVReader & reader)
{
	const spv::Id structType = reader.ReadWord();
	const spv::Id idx = reader.ReadWord();
	const string str = reader.ReadLiteralString();

	if (memberNames.find(structType) != memberNames.end())
	{
		/* Make sure we have enough space. */
		memberNames[structType].resize(idx + 1);
		memberNames[structType][idx] = str;
	}
	else
	{
		/* If we cannot find the containing struct yet, just add it. */
		vector<string> members = { str };
		memberNames.emplace(structType, members);
	}
}

void Pu::Shader::HandleDecorate(SPIRVReader & reader)
{
	/* Read the target and create the decoration object. */
	const spv::Id target = reader.ReadWord();
	const spv::Decoration decoration = _CrtInt2Enum<spv::Decoration>(reader.ReadWord());

	Decoration result;
	switch (decoration)
	{
	case (spv::Decoration::Block):			// Used for uniform blocks we don't have to define what is in which block.
		break;
	case (spv::Decoration::Location):		// Location decoration stores a single literal number.
		result.Numbers.emplace(decoration, reader.ReadWord());
		break;
	case (spv::Decoration::DescriptorSet):	// Descriptor set decoration stores a single literal number.
		result.Numbers.emplace(decoration, reader.ReadWord());
		break;
	case (spv::Decoration::Binding):		// Binding decoration stores a single literal number.
		result.Numbers.emplace(decoration, reader.ReadWord());
		break;
	default:								// Just log that we don't hanle the decoration at this point.
		Log::Warning("Unable to handle decoration type '%s' at this point!", to_string(decoration));
		return;
	}

	/* Only push if the decoration type was handled, also merge them together if the target is already added. */
	if (decorations.find(target) != decorations.end()) decorations[target].Merge(result);
	else decorations.emplace(target, result);
}

void Pu::Shader::HandleType(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	reader.AdvanceWord();	// storage class.
	typedefs.emplace(id, reader.ReadWord());
}

void Pu::Shader::HandleInt(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const spv::Word width = reader.ReadWord();
	const bool isSigned = reader.ReadWord();

	FieldType intType;
	if (width == 8) intType.ComponentType = isSigned ? ComponentType::SByte : ComponentType::Byte;
	else if (width == 16) intType.ComponentType = isSigned ? ComponentType::Short : ComponentType::UShort;
	else if (width == 32) intType.ComponentType = isSigned ? ComponentType::Int : ComponentType::UInt;
	else if (width == 64) intType.ComponentType = isSigned ? ComponentType::Long : ComponentType::ULong;

	types.emplace(id, intType);
}

void Pu::Shader::HandleFloat(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const spv::Word width = reader.ReadWord();

	FieldType floatType;
	if (width == 16) floatType.ComponentType = ComponentType::HalfFloat;
	else if (width == 32) floatType.ComponentType = ComponentType::Float;
	else if (width == 64) floatType.ComponentType = ComponentType::Double;

	types.emplace(id, floatType);
}

void Pu::Shader::HandleVector(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const FieldType componentType = types[reader.ReadWord()];
	const spv::Word componentCnt = reader.ReadWord();

	SizeType sizeType;
	if (componentCnt == 2) sizeType = SizeType::Vector2;
	else if (componentCnt == 3) sizeType = SizeType::Vector3;
	else if (componentCnt == 4) sizeType = SizeType::Vector4;
	else
	{
		Log::Warning("Invalid vector dimensions (%d) defined in SPIR-V, ignoring vector!", componentCnt);
		return;
	}

	types.emplace(id, FieldType(componentType.ComponentType, sizeType));
}

void Pu::Shader::HandleMatrix(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	const FieldType columnType = types[reader.ReadWord()];
	const spv::Word columnCnt = reader.ReadWord();

	switch (columnType.ContainerType)
	{
	case Pu::SizeType::Vector2:
		if (columnCnt == 2) types.emplace(id, FieldType(columnType.ComponentType, SizeType::Matrix2));
		else Log::Warning("Unable to handle non-square matrices!");
		break;
	case Pu::SizeType::Vector3:
		if (columnCnt == 3) types.emplace(id, FieldType(columnType.ComponentType, SizeType::Matrix3));
		else Log::Warning("Unable to handle non-square matrices!");
		break;
	case Pu::SizeType::Vector4:
		if (columnCnt == 4) types.emplace(id, FieldType(columnType.ComponentType, SizeType::Matrix4));
		else Log::Warning("Unable to handle non-square matrices!");
		break;
	default:
		Log::Warning("'%s' is invalid for %uD matrix conponent type!", columnType.GetName().c_str(), columnCnt, columnCnt);
		break;
	}
}

void Pu::Shader::HandleStruct(SPIRVReader & reader, size_t memberCnt)
{
	const spv::Id id = reader.ReadWord();
	vector<spv::Id> members;
	members.reserve(memberCnt);
	for (size_t i = 0; i < memberCnt; i++) members.emplace_back(reader.ReadWord());

	structs.emplace(id, std::move(members));
}

void Pu::Shader::HandleImage(SPIRVReader & reader)
{
	/* We need the type id but we can skip the underlying image type as it doesn't concern us. */
	const spv::Id id = reader.ReadWord();
	reader.AdvanceWord();

	switch (_CrtInt2Enum<spv::Dim>(reader.ReadWord()))
	{
	case (spv::Dim::Dim1D):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Scalar));
		break;
	case (spv::Dim::Dim2D):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Vector2));
		break;
	case (spv::Dim::Dim3D):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Vector3));
		break;
	case (spv::Dim::Cube):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Cube));
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
}

void Pu::Shader::HandleSampledImage(SPIRVReader & reader)
{
	/* Just add the type of the image that's being sampled to the type list. */
	const spv::Id id = reader.ReadWord();
	types.emplace(id, types[reader.ReadWord()]);
}

void Pu::Shader::HandleVariable(SPIRVReader & reader)
{
	const spv::Id resultType = reader.ReadWord();
	const spv::Id resultId = reader.ReadWord();
	const spv::StorageClass storageClass = _CrtInt2Enum<spv::StorageClass>(reader.ReadWord());

	variables.emplace_back(std::make_tuple(resultId, resultType, storageClass));
}

void Pu::Shader::SetInfo(const wstring & ext)
{
	if (ext == L"VERT") info.Stage = ShaderStageFlag::Vertex;
	else if (ext == L"TESC") info.Stage = ShaderStageFlag::TessellationControl;
	else if (ext == L"TESE") info.Stage = ShaderStageFlag::TessellationEvaluation;
	else if (ext == L"GEOM") info.Stage = ShaderStageFlag::Geometry;
	else if (ext == L"FRAG") info.Stage = ShaderStageFlag::Fragment;
	else if (ext == L"COMP") info.Stage = ShaderStageFlag::Compute;
}

void Pu::Shader::Destroy(void)
{
	if (info.Module) parent.vkDestroyShaderModule(parent.hndl, info.Module, nullptr);
}

Pu::Shader::LoadTask::LoadTask(Shader & result, const wstring & path)
	: result(result), path(path)
{
	result.SetHash(std::hash<wstring>{}(path));
}

Pu::Task::Result Pu::Shader::LoadTask::Execute(void)
{
	result.Load(path, true);
	return Result::Default();
}