#pragma once
#include <Graphics/Models/UniformBlock.h>
#include <Graphics/Vulkan/Shaders/GraphicsPipeline.h>
#include <Content/PumLoader.h>

class MonsterMaterial
	: public Pu::UniformBlock
{
public:
	MonsterMaterial(const Pu::Subpass &subpass, Pu::DescriptorPool &pool)
		: UniformBlock(subpass, pool, { "Glossiness", "F0", "DiffuseFactor" }),
		albedoDescriptor(subpass.GetDescriptor("Diffuse"))
	{}

	inline void SetParameters(float glossiness, Pu::Vector3 specular, Pu::Vector3 diffuse, const Pu::Texture2D &diffuseMap)
	{
		this->glossiness = glossiness;
		this->specular = specular;
		this->diffuse = diffuse;
		IsDirty = true;

		Write(albedoDescriptor, diffuseMap);
	}

	inline void SetParameters(const Pu::PumMaterial &material, const Pu::Texture2D &albedo)
	{
		SetParameters(material.Glossiness, material.SpecularFactor.ToVector3(), material.DiffuseFactor.ToVector3(), albedo);
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