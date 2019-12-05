#pragma once
#include <Graphics/Models/UniformBlock.h>

class TransformBlock
	: public Pu::UniformBlock
{
public:
	TransformBlock(Pu::DescriptorPool &pool)
		: UniformBlock(pool, false)
	{
		offset = pool.GetSubpass().GetDescriptor("CamPos").GetAllignedOffset(sizeof(Pu::Matrix) * 2);
	}

	inline void SetProjection(const Pu::Matrix &mtrx)
	{
		proj = mtrx;
		IsDirty = true;
	}

	inline void SetView(const Pu::Matrix &mtrx)
	{
		view = mtrx;
		IsDirty = true;
	}

	inline void SetCamPos(Pu::Vector3 v)
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