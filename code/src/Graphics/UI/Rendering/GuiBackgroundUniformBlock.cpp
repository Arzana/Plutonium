#include "Graphics/UI/Rendering/GuiBackgroundUniformBlock.h"

Pu::GuiBackgroundUniformBlock::GuiBackgroundUniformBlock(LogicalDevice & device, const GraphicsPipeline & pipeline)
	: UniformBlock(device, pipeline, { "Model", "Color", "Border", "Size", "Position" })
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

void Pu::GuiBackgroundUniformBlock::SetBorder(float value)
{
	border = value;
	IsDirty = true;
}

void Pu::GuiBackgroundUniformBlock::SetSize(Vector2 value)
{
	size = value;
	IsDirty = true;
}

void Pu::GuiBackgroundUniformBlock::SetPosition(Vector2 value)
{
	position = value;
	IsDirty = true;
}

void Pu::GuiBackgroundUniformBlock::Stage(byte * dest)
{
	Copy(dest, model.GetComponents());
	size_t offset = allignedOffset;

	Copy(dest + offset, &color);
	offset += sizeof(Vector4);

	Copy(dest + offset, &border);
	offset += sizeof(float);

	Copy(dest + offset, &size);
	offset += sizeof(Vector2);

	Copy(dest + offset, &position);
}