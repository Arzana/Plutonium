#pragma once
#include <Graphics/Models/UniformBlock.h>
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>

class TransformBlock
	: public Pu::UniformBlock
{
public:
	TransformBlock(_In_ const Pu::Subpass &subpass, Pu::DescriptorPool &pool)
		: UniformBlock(subpass, pool, { "Projection", "View", "Model", "CamPos" })
	{
		offset = subpass.GetDescriptor("CamPos").GetAllignedOffset(sizeof(Pu::Matrix) * 3);
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

	inline void SetModel(_In_ const Pu::Matrix &mtrx)
	{
		mdl = mtrx;
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
		Copy(dest + sizeof(Pu::Matrix) * 2, &mdl);
		Copy(dest + offset, &camPos);
	}

private:
	size_t offset;
	Pu::Matrix proj, view, mdl;
	Pu::Vector3 camPos;
};