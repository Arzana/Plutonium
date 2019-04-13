#include "Graphics/Text/TextUniformBlock.h"

Pu::TextUniformBlock::TextUniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline)
	: UniformBlock(device, sizeof(Vector4), pipeline.GetDescriptorPool(), 0),
	uniTex(pipeline.GetRenderpass().GetUniform("Atlas")),
	uniClr(pipeline.GetRenderpass().GetUniform("Color"))
{}

Pu::TextUniformBlock::TextUniformBlock(TextUniformBlock && value)
	: UniformBlock(std::move(value)), uniTex(std::move(value.uniTex)), uniClr(std::move(value.uniClr))
{}

void Pu::TextUniformBlock::SetColor(Color color)
{
	clr = color.ToVector4();
	IsDirty = true;
}

void Pu::TextUniformBlock::SetAtlas(const Texture2D & atlas)
{
	GetDescriptor().Write(uniTex, atlas);
}

void Pu::TextUniformBlock::Stage(byte * dest)
{
	memcpy(dest, &clr, sizeof(Vector4));
}

void Pu::TextUniformBlock::UpdateDescriptor(DescriptorSet & set, const Buffer & uniformBuffer)
{
	set.Write({ &uniClr }, uniformBuffer);
}