#include "Graphics/Vulkan/SPIR-V/Subpass.h"
#include "Streams/FileUtils.h"
#include "Streams/FileReader.h"
#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"

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
	if (FileReader::FileExists(path.c_str()))
	{
		const string code = FileReader(path.c_str()).ReadToEnd();
		ShaderModuleCreateInfo createInfo(code);
		VK_VALIDATE(parent.vkCreateShaderModule(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateShaderModule);
	}
	else Log::Fatal("Unable to load shader module!");
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