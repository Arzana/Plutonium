#pragma once
#include <Graphics/Models/UniformBlock.h>
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>

class TransformBlock
	: public Pu::UniformBlock
{
public:
	TransformBlock(_In_ Pu::LogicalDevice &device, _In_ const Pu::GraphicsPipeline &pipeline)
		: UniformBlock(device, sizeof(Pu::Matrix) * 2, pipeline.GetDescriptorPool(), 0), 
		uniProj(pipeline.GetRenderpass().GetUniform("Projection")),
		uniView(pipeline.GetRenderpass().GetUniform("View"))
	{}

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

	inline void SetTexture(_In_ const Pu::Uniform &texture, _In_ const Pu::Texture2D &resource)
	{
		GetDescriptor().Write(texture, resource);
	}

protected:

	virtual inline void Stage(Pu::byte *dest) override
	{
		memcpy(dest, proj.GetComponents(), sizeof(Pu::Matrix));
		memcpy(dest + sizeof(Pu::Matrix), view.GetComponents(), sizeof(Pu::Matrix));
	}

	virtual inline void UpdateDescriptor(Pu::DescriptorSet &descriptor, const Pu::Buffer &uniformBuffer) override
	{
		descriptor.Write({ &uniProj, &uniView }, uniformBuffer);
	}

private:
	Pu::Matrix proj, view;
	const Pu::Uniform &uniProj, &uniView;
};