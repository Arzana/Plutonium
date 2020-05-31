#include "Graphics/Vulkan/Shaders/Shader.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"

const Pu::FieldInfo Pu::Shader::invalid = Pu::FieldInfo();
constexpr spv::Word GlobalMemberIndex = ~0u;
Pu::SpecializationConstant Pu::Shader::defConst = Pu::SpecializationConstant(0, "", Pu::FieldType(Pu::ComponentType::Invalid, Pu::SizeType::Scalar));

Pu::Shader::Shader(LogicalDevice & device)
	: Asset(true), parent(&device)
{}

Pu::Shader::Shader(LogicalDevice & device, const wstring & path)
	: Asset(true, std::hash<wstring>{}(path)), parent(&device)
{
	Load(path, false);
}

Pu::Shader::Shader(LogicalDevice & device, const void *src, size_t size, ShaderStageFlag stage)
	: Asset(true), parent(&device)
{
	/* Create a new reader and immediately load the shader. */
	SPIRVReader spvr{ src, size };
	Create(spvr);

	/* Set the shader information. */
	info.Stage = stage;
	SetFieldInfo();

	/* Mark the asset as loaded and set a default name. */
	wstring name = L"Raw SPIR-V ";
	name += string(to_string(stage)).toWide();
	name += L" shader";
	MarkAsLoaded(false, std::move(name));
}

Pu::Shader::Shader(Shader && value)
	: Asset(std::move(value)), parent(value.parent), info(value.info),
	fields(std::move(value.fields)), specializationConstants(std::move(value.specializationConstants))
{
	value.info.Module = nullptr;
}

Pu::Shader & Pu::Shader::operator=(Shader && other)
{
	if (this != &other)
	{
		Destroy();

		Asset::operator=(std::move(other));
		parent = other.parent;
		info = other.info;
		fields = std::move(other.fields);
		specializationConstants = std::move(other.specializationConstants);

		other.info.Module = nullptr;
	}

	return *this;
}

Pu::SpecializationConstant & Pu::Shader::GetConstant(const string & name)
{
	for (SpecializationConstant &cur : specializationConstants)
	{
		if (name == cur.name) return cur;
	}

	Log::Error("Unable to find specialization constant '%s' in shader '%ls'!", name.c_str(), GetName().c_str());
	return defConst;
}

const Pu::SpecializationConstant & Pu::Shader::GetConstant(const string & name) const
{
	for (const SpecializationConstant &cur : specializationConstants)
	{
		if (name == cur.name) return cur;
	}

	Log::Error("Unable to find specialization constant '%s' in shader '%ls'!", name.c_str(), GetName().c_str());
	return defConst;
}

Pu::Asset & Pu::Shader::Duplicate(AssetCache &)
{
	Reference();
	return *this;
}

void Pu::Shader::Load(const wstring & path, bool viaLoader)
{
	const wstring ext = path.fileExtension().toUpper();

	if (ext == L"SPV")
	{
		/* Crash early if the shader cannot be loaded. */
		if (!FileReader::FileExists(path)) Log::Fatal("Unable to load shader module '%ls'!", path.fileName().c_str());

		/* If the input shader is already defined as binary just load it. */
		SPIRVReader spvr{ path };
		Create(spvr);
		SetInfo(path.substr(0, path.length() - 4).fileExtension().toUpper());
	}
	else Log::Fatal("'%ls' cannot be loaded as a shader (only SPIR-V shaders are valid)!", ext.c_str());

	/* Set the information of the subpass. */
	SetFieldInfo();
	MarkAsLoaded(viaLoader, path.fileName());
}

void Pu::Shader::Create(SPIRVReader & reader)
{
	/* Compile the SPIR-V shader module. */
	ShaderModuleCreateInfo createInfo(reader.GetStream().GetSize(), reader.GetStream().GetData());
	VK_VALIDATE(parent->vkCreateShaderModule(parent->hndl, &createInfo, nullptr, &info.Module), PFN_vkCreateShaderModule);

	/* Perform reflection to get the inputs and outputs. */
	auto handler = DelegateMethod<SPIRVReader, Shader, spv::Op, size_t>(*this, &Shader::HandleModule);
	reader.HandleAllModules(handler);
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
	constants.clear();
}

void Pu::Shader::HandleVariable(spv::Id id, spv::Id typeId, spv::StorageClass storage)
{
	const spv::Id typePointer = typedefs[typeId];

	/* Handle normal type. */
	if (types.find(typePointer) != types.end())
	{
		/* User defined variables need to have at least one handlable decoration, all others must be build in variables, which we can skip. */
		const DecoratePair key{ id, GlobalMemberIndex };
		if (decorations.find(key) != decorations.end())
		{
			/* Only handle types with defined names, this should never occur. */
			fields.emplace_back(id, std::move(names[id]), std::move(types[typePointer]), storage, decorations[key]);
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
				const DecoratePair globalKey{ id, GlobalMemberIndex };
				const DecoratePair memberKey{ typePointer, static_cast<spv::Word>(i) };

				Decoration memberDecoration(offset);
				memberDecoration.Merge(decorations[globalKey]);
				memberDecoration.Merge(decorations[memberKey]);

				fields.emplace_back(memberTypeId, std::move(memberName), std::move(fieldType), storage, memberDecoration);
			}
			else if (fieldType.ComponentType == ComponentType::Invalid)
			{
				/* This occurs if the user didn't create a gl_PerVertex block, so we hardset gl_ variables to their defaults. */
				if (memberName == "gl_Position") offset += sizeof(Vector4);
				else if (memberName == "gl_PointSize") offset += sizeof(float);
				else if (memberName == "gl_ClipDistance") offset += sizeof(float);
				else if (memberName == "gl_CullDistance") offset += sizeof(float);
				else Log::Fatal("Cannot extract byte size of '%s', was gl_PerVertex not used?", memberName.c_str());
			}
			else
			{
				/* Increase the offset. */
				offset += fieldType.GetSize();
			}
		}
	}
	else Log::Warning("Unable to handle SPIR-V %s '%s'(%u)!", to_string(storage), names[id].c_str(), id);
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
	case (spv::Op::OpMemberDecorate):
		HandleMemberDecorate(reader);
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
	case (spv::Op::OpTypeArray):
		HandleArray(reader);
		break;
	case (spv::Op::OpTypeImage):
		HandleImage(reader);
		break;
	case (spv::Op::OpTypeSampledImage):
		HandleSampledImage(reader);
		break;
	case (spv::Op::OpVariable):
		HandleVariable(reader);
		break;
	case (spv::Op::OpConstant):
		HandleConstant(reader);
		break;
	case (spv::Op::OpSpecConstant):
		HandleSpecConstant(reader);
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
	HandleDecoration(reader, target, GlobalMemberIndex, decoration);
}

void Pu::Shader::HandleMemberDecorate(SPIRVReader & reader)
{
	/* We get the target struct and member index from the SPIR-V source. */
	const spv::Id structType = reader.ReadWord();
	const spv::Word member = reader.ReadWord();

	const spv::Decoration decoration = _CrtInt2Enum<spv::Decoration>(reader.ReadWord());
	HandleDecoration(reader, structType, member, decoration);
}

void Pu::Shader::HandleDecoration(SPIRVReader & reader, spv::Id target, spv::Word idx, spv::Decoration decoration)
{
	Decoration result;
	switch (decoration)
	{
	case (spv::Decoration::Block):					// Used for uniform blocks we don't have to define what is in which block.
	case (spv::Decoration::Flat):					// Used for internal shader inputs (should not be interpolated), we cannot access this variable anyway.
	case (spv::Decoration::BuiltIn):				// Used to indicate build in variables, they'll not be used.
	case (spv::Decoration::Patch):					// Used to indicate that the I/O field uses patches (we don't care about this).
	case (spv::Decoration::ArrayStride):			// Used to specify the stride of arrays, this is currently only used in geometry shading.
	case (spv::Decoration::ColMajor):				// Plutonium expects column major matrices.
	case (spv::Decoration::MatrixStride):			// Matrix stride is determined when a matrix type if found.
		return;
	case (spv::Decoration::Location):				// Location decoration stores a single literal number.
	case (spv::Decoration::DescriptorSet):			// Descriptor set decoration stores a single literal number.
	case (spv::Decoration::Binding):				// Binding decoration stores a single literal number.
	case (spv::Decoration::InputAttachmentIndex):	// Describes the index of the input attachment in the framebuffer.
	case (spv::Decoration::Offset):					// Offset is used for struct offsets (mainly descriptors).
	case (spv::Decoration::SpecId):					// Specialization ID's are used to access pipeline constant values.
		result.Numbers.emplace(decoration, reader.ReadWord());
		break;
	case (spv::Decoration::RowMajor):
		Log::Error("Plutonium uses column-major matrices, field %u uses row-major!", target);
		return;
	default:										// Just log that we don't hanle the decoration at this point.
		Log::Warning("Unable to handle decoration type '%s' at this point!", to_string(decoration));
		return;
	}

	/* Only push if the decoration type was handled, also merge them together if the target is already added. */
	const DecoratePair key{ target, idx };
	if (decorations.find(key) != decorations.end()) decorations[key].Merge(result);
	else decorations.emplace(key, result);
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
	else Log::Warning("Invalid integer type length found in SPIR-V!");

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
	else Log::Warning("Invalid float type length found in SPIR-V!");

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

void Pu::Shader::HandleArray(SPIRVReader & reader)
{
	const spv::Id id = reader.ReadWord();
	FieldType elementType = types[reader.ReadWord()];
	elementType.Length = static_cast<spv::Word>(constants[reader.ReadWord()]);

	types.emplace(id, elementType);
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
	case (spv::Dim::SubpassData):		// Input attachments are always 2D.
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Vector2));
		break;
	case (spv::Dim::Dim3D):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Vector3));
		break;
	case (spv::Dim::Cube):
		types.emplace(id, FieldType(ComponentType::Image, SizeType::Cube));
		break;
	default:
		Log::Warning("Unable to handle SPIR-V image (unhandled dimension)!");
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

void Pu::Shader::HandleConstant(SPIRVReader & reader)
{
	FieldType type = types[reader.ReadWord()];
	const spv::Id resultId = reader.ReadWord();

	if (type.ContainerType == SizeType::Scalar)
	{
		constants.emplace(resultId, reader.ReadComponentType(type.ComponentType));
	}
	else Log::Error("Non-scalar constrant defined in SPIR-V, ignoring constant!");
}

void Pu::Shader::HandleSpecConstant(SPIRVReader & reader)
{
	/* The type and decoration are already defined before this specialization constant is defined. */
	FieldType type = types[reader.ReadWord()];
	const spv::Id id = reader.ReadWord();
	Decoration &decoration = decorations[std::make_pair(id, GlobalMemberIndex)];

	/* The default value for this constant is stored at the end of this sub-stream. */
	SpecializationConstant value{ id, names[id], type };
	value.entry.ConstantID = decoration.Numbers[spv::Decoration::SpecId];
	value.defaultValue = reader.ReadComponentType(type.ComponentType);

	specializationConstants.emplace_back(std::move(value));
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
	if (info.Module) parent->vkDestroyShaderModule(parent->hndl, info.Module, nullptr);
}

Pu::Shader::LoadTask::LoadTask(Shader & result, const wstring & path)
	: result(result), path(path)
{}

Pu::Task::Result Pu::Shader::LoadTask::Execute(void)
{
	result.Load(path, true);
	return Result::Default();
}