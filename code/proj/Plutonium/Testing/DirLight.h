#pragma once
#include <Graphics/Models/UniformBlock.h>

class DirLight
	: public Pu::UniformBlock
{
public:
	DirLight(Pu::DescriptorPool &pool)
		: UniformBlock(pool, true), dir(1.0f, 0.0f, 0.0f), radiance(1.0f), intensity(1.0f)
	{}

	inline void SetDirection(Pu::Vector3 direction)
	{
		dir = direction.Normalize();
		IsDirty = true;
	}

	inline void SetRadiance(Pu::Color color)
	{
		radiance = color.ToVector3();
		IsDirty = true;
	}

	inline void SetIntensity(float value)
	{
		intensity = value;
		IsDirty = true;
	}

	inline Pu::Vector3 GetDirection(void) const
	{
		return dir;
	}

	inline Pu::Vector3 GetRadiance(void) const
	{
		return radiance;
	}

	inline float GetIntensity(void) const
	{
		return intensity;
	}

protected:
	virtual inline void Stage(Pu::byte *dest) override
	{
		Copy(dest, &dir);
		Copy(dest + sizeof(Pu::Vector4), &radiance);
		Copy(dest + sizeof(Pu::Vector4) + sizeof(Pu::Vector3), &intensity);
	}

private:
	Pu::Vector3 dir;
	Pu::Vector3 radiance;
	float intensity;
};