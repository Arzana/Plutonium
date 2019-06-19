#pragma once
#include <Graphics/Models/UniformBlock.h>
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Content/PumLoader.h>

class MonsterMaterial
	: public Pu::UniformBlock
{
public:
	MonsterMaterial(const Pu::GraphicsPipeline &pipeline)
		: UniformBlock(pipeline, { "Glossiness", "F0", "DiffuseFactor" }),
		albedoDescriptor(pipeline.GetRenderpass().GetDescriptor("Diffuse"))
	{}

	inline void SetParameters(const Pu::PumMaterial &material, const Pu::Texture2D &albedo)
	{
		glossiness = material.Glossiness;
		specular = material.SpecularFactor.ToVector4().XYZ;
		diffuse = material.DiffuseFactor.ToVector4().XYZ;
		IsDirty = true;

		Write(albedoDescriptor, albedo);
	}

protected:
	virtual inline void Stage(Pu::byte *dest) override
	{
		Copy(dest, &specular);
		Copy(dest + sizeof(Pu::Vector3), &diffuse);
		Copy(dest + sizeof(Pu::Vector3) + sizeof(Pu::Vector3), &glossiness);
	}

private:
	float glossiness;
	Pu::Vector3 specular;
	Pu::Vector3 diffuse;
	const Pu::Descriptor &albedoDescriptor;
};