#include "Graphics/Diagnostics/DebugRendererUniformBlock.h"

Pu::DebugRendererUniformBlock::DebugRendererUniformBlock(const Subpass & subpass, DescriptorPool & pool)
	: UniformBlock(subpass, pool, { "Projection", "View" })
{}

Pu::DebugRendererUniformBlock::DebugRendererUniformBlock(DebugRendererUniformBlock && value)
	: UniformBlock(std::move(value)), proj(value.proj), view(value.view)
{}

Pu::DebugRendererUniformBlock & Pu::DebugRendererUniformBlock::operator=(DebugRendererUniformBlock && other)
{
	if (this != &other)
	{
		UniformBlock::operator=(std::move(other));
		proj = other.proj;
		view = other.view;
	}

	return *this;
}

void Pu::DebugRendererUniformBlock::SetProjection(const Matrix & value)
{
	proj = value;
	IsDirty = true;
}

void Pu::DebugRendererUniformBlock::SetView(const Matrix & value)
{
	view = value;
	IsDirty = true;
}

void Pu::DebugRendererUniformBlock::Stage(byte * dest)
{
	Copy(dest, &proj);
	Copy(dest + sizeof(Matrix), &view);
}