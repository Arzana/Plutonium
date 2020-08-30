#include "Graphics/Vulkan/Shaders/Subpass.h"

using namespace Pu;

FieldInfo Subpass::dsInfo = FieldInfo(0, "DepthStencil", FieldType(ComponentType::Float, SizeType::Scalar), spv::StorageClass::Output, Decoration());

Pu::Subpass::Subpass(LogicalDevice & device, const vector<Shader*>& shaderModules)
	: ShaderProgram(device, shaderModules)
{}

void Pu::Subpass::AddDependency(uint32 srcSubpass, PipelineStageFlags srcStage, PipelineStageFlags dstStage, AccessFlags srcAccess, AccessFlags dstAccess, DependencyFlags flags)
{
	/* Destination subpass is set by the renderpass. */
	SubpassDependency dependency{ srcSubpass, SubpassNotSet };
	dependency.SrcStageMask = srcStage;
	dependency.SrcAccessMask = srcAccess;
	dependency.DstStageMask = dstStage;
	dependency.DstAccessMask = dstAccess;
	dependency.DependencyFlags = flags;

	dependencies.emplace_back(dependency);
}

Output & Subpass::AddDepthStencil(void)
{
	/* Make sure we don't attach multiple depth/stencil attachment. */
	if (ds) return *ds;

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

void Pu::Subpass::CloneOutput(const string & name, uint32 referenceIndex)
{
	Output &output = GetOutput(name);
	output.SetReference(referenceIndex);
	output.clone = true;
}