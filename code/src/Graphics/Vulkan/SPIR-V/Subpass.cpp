#include "Graphics/Vulkan/SPIR-V/Subpass.h"
#include "Streams/FileUtils.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"

Pu::Subpass::Subpass(LogicalDevice & device, const char * path)
	: parent(device)
{
	const string ext = _CrtGetFileExtension(path);
	if (ext == "spv")
	{
		/* If the input shader is already defined as binary just load it. */
		Create(path);
		SetStage(_CrtGetFileExtension(string(path, strlen(path) - 4)));
	}
	else
	{
		/* First compile the shader to SPIR-V and then load it. */
		Create(SPIRV::FromGLSLPath(path));
		SetStage(ext);
	}
}

Pu::Subpass::Subpass(Subpass && value)
	: parent(value.parent), hndl(value.hndl), stage(value.stage)
{
	value.hndl = nullptr;
}

Pu::Subpass & Pu::Subpass::operator=(Subpass && other)
{
	if (this != &other)
	{
		Destroy();
		parent = std::move(other.parent);
		hndl = other.hndl;
		stage = other.stage;

		other.hndl = nullptr;
	}

	return *this;
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
		VK_VALIDATE(parent.vkCreateShaderModule(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateShaderModule);

		/* Perform reflection to get the inputs and outputs. */
		spvr.HandleAllModules(SPIRVReader::ModuleHandler(*this, &Subpass::HandleModule));
	}
	else Log::Fatal("Unable to load shader module!");
}

void Pu::Subpass::HandleModule(SPIRVReader & reader, spv::Op opCode, size_t)
{
	/* Pass usefull operation codes to their functions. */
	switch (opCode)
	{
	case (spv::Op::OpName):
		HandleName(reader);
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
	const string name = reader.ReadLiteralString();
	names.emplace(target, name);
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

void Pu::Subpass::SetStage(const string & ext)
{
	if (ext == "vert") stage = ShaderStageFlag::Vertex;
	else if (ext == "tesc") stage = ShaderStageFlag::TessellationControl;
	else if (ext == "tese") stage = ShaderStageFlag::TessellationEvaluation;
	else if (ext == "geom") stage = ShaderStageFlag::Geometry;
	else if (ext == "frag") stage = ShaderStageFlag::Fragment;
	else if (ext == "comp") stage = ShaderStageFlag::Compute;
	else stage = ShaderStageFlag::Unknown;
}

void Pu::Subpass::Destroy(void)
{
	if (hndl) parent.vkDestroyShaderModule(parent.hndl, hndl, nullptr);
}