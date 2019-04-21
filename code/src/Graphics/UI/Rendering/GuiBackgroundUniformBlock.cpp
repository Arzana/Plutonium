#include "Graphics/UI/Rendering/GuiBackgroundUniformBlock.h"

Pu::GuiBackgroundUniformBlock::GuiBackgroundUniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline)
	: UniformBlock(device, pipeline, { "Model", "Color" })
{
	allignedOffset = pipeline.GetRenderpass().GetUniform("Color").GetAllignedOffset(sizeof(Matrix));
}

Pu::GuiBackgroundUniformBlock::GuiBackgroundUniformBlock(GuiBackgroundUniformBlock && value)
	: UniformBlock(std::move(value)), allignedOffset(value.allignedOffset)
{}

void Pu::GuiBackgroundUniformBlock::SetModel(const Matrix & value)
{
	model = value;
	IsDirty = true;
}

void Pu::GuiBackgroundUniformBlock::SetColor(Color value)
{
	color = value.ToVector4();
	IsDirty = true;
}

void Pu::GuiBackgroundUniformBlock::Stage(byte * dest)
{
	Copy(dest, &model);
	size_t offset = allignedOffset;

	Copy(dest + offset, &color);
	offset += sizeof(Vector4);
}