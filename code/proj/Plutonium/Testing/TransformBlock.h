#pragma once
#include <Graphics/Models/UniformBlock.h>
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>

class TransformBlock
	: public Pu::UniformBlock
{
public:
	TransformBlock(_In_ const Pu::GraphicsPipeline &pipeline)
		: UniformBlock(pipeline, { "Projection", "View", "CamPos" })
	{
		offset = pipeline.GetRenderpass().GetDescriptor("CamPos").GetAllignedOffset(sizeof(Pu::Matrix) << 1);
	}

	inline void SetProjection(_In_ const Pu::Matrix &mtrx)
	{
		proj = mtrx;
		IsDirty = true;
	}

	inline void SetView(_In_ const Pu::Matrix &mtrx)
	{
		view = mtrx;
		IsDirty = true;
	}

	inline void SetCamPos(_In_ Pu::Vector3 v)
	{
		camPos = v;
		IsDirty = true;
	}

protected:
	virtual inline void Stage(Pu::byte *dest) override
	{
		Copy(dest, &proj);
		Copy(dest + sizeof(Pu::Matrix), &view);
		Copy(dest + offset, &camPos);
	}

private:
	size_t offset;
	Pu::Matrix proj, view;
	Pu::Vector3 camPos;
};