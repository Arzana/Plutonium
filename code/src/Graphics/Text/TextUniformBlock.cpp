#include "Graphics/Text/TextUniformBlock.h"

/*
The shader has two descriptor sets.
The first one (0) is for constants across the shader (i.e. the atlas and projection matrix).
The second one (1) is for the string specific data (i.e. the transform and color).
*/
Pu::TextUniformBlock::TextUniformBlock(_In_ DescriptorPool & pool)
	: UniformBlock(pool)
{
	allignedOffset = pool.GetSubpass().GetDescriptor("Color").GetAllignedOffset(sizeof(Matrix));
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
	/* 
	The model matrix is stored in binding 0 and the color is stored in binding 1.
	This is because they need to be used in different shaders.
	For this we need to make sure that the color is alligned properly.
	*/
	Copy(dest, &model);
	Copy(dest + allignedOffset, &clr);
}