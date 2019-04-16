#include "Graphics/Text/ConstTextUniformBlock.h"

/*
The shader has two descriptor sets.
The first one (0) is for constants across the shader (i.e. the atlas and projection matrix).
The second one (1) is for the string specific data (i.e. the transform and color).
*/
Pu::ConstTextUniformBlock::ConstTextUniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline)
	: UniformBlock(device, pipeline, { "Projection" }), uniTex(pipeline.GetRenderpass().GetUniform("Atlas"))
{}

Pu::ConstTextUniformBlock::ConstTextUniformBlock(ConstTextUniformBlock && value)
	: UniformBlock(std::move(value)), uniTex(std::move(value.uniTex))
{}

void Pu::ConstTextUniformBlock::SetProjection(const Matrix & matrix)
{
	proj = matrix;
	IsDirty = true;
}

void Pu::ConstTextUniformBlock::SetAtlas(const Texture2D & texture)
{
	Write(uniTex, texture);
}

void Pu::ConstTextUniformBlock::Stage(byte * dest)
{
	memcpy(dest, proj.GetComponents(), sizeof(Matrix));
}