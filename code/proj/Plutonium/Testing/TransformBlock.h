#pragma once
#include <Graphics/Models/UniformBlock.h>

class TransformBlock
	: public Pu::UniformBlock
{
public:
	TransformBlock(_In_ Pu::LogicalDevice &device, _In_ const Pu::DescriptorPool &pool, _In_ Pu::uint32 set, _In_ const Pu::Uniform &projection)
		: UniformBlock(device, sizeof(Pu::Matrix), pool, set), uniform(projection)
	{}

	inline void SetProjection(_In_ const Pu::Matrix &mtrx)
	{
		projection = mtrx;
		IsDirty = true;
	}

	inline void SetTexture(_In_ const Pu::Uniform &texture, _In_ const Pu::Texture2D &resource)
	{
		GetDescriptor().Write(texture, resource);
	}

protected:

	virtual inline void Stage(Pu::StagingBuffer &dest) override
	{
		dest.Load(projection.GetComponents());
	}

	virtual inline void UpdateDescriptor(Pu::DescriptorSet &descriptor, const Pu::Buffer &uniformBuffer) override
	{
		descriptor.Write(uniform, uniformBuffer);
	}

private:
	Pu::Matrix projection;
	const Pu::Uniform &uniform;
};