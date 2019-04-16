#include "Graphics/Text/TextUniformBlock.h"

/*
The shader has two descriptor sets.
The first one (0) is for constants across the shader (i.e. the atlas and projection matrix).
The second one (1) is for the string specific data (i.e. the transform and color).
*/
Pu::TextUniformBlock::TextUniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline)
	: UniformBlock(device, pipeline, { "Model", "Color" })
{
	allignedOffset = pipeline.GetRenderpass().GetUniform("Color").GetAllignedOffset(sizeof(Matrix));
}

Pu::TextUniformBlock::TextUniformBlock(TextUniformBlock && value)
	: UniformBlock(std::move(value)), allignedOffset(value.allignedOffset)
{}

void Pu::TextUniformBlock::SetColor(Color color)
{
	clr = color.ToVector4();
	IsDirty = true;
}

void Pu::TextUniformBlock::SetModel(const Matrix & matrix)
{
	model = matrix;
	IsDirty = true;
}

void Pu::TextUniformBlock::Stage(byte * dest)
{
	memcpy(dest, model.GetComponents(), sizeof(Matrix));
	memcpy(dest + allignedOffset, &clr, sizeof(Vector4));
}