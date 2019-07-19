#include "Graphics/Models/Material.h"

Pu::Material::Material(const Subpass & subpass, DescriptorPool & pool)
	: UniformBlock(subpass, pool, { "F0", "DiffuseFactor", "Glossiness" }), 
	diffuseMap(&subpass.GetDescriptor("Diffuse")), roughness(1.0f)
{}

Pu::Material::Material(Material && value)
	: UniformBlock(std::move(value)), diffuseMap(value.diffuseMap),
	roughness(value.roughness), f0(value.f0), diffuse(value.diffuse)
{
	value.diffuseMap = nullptr;
}

Pu::Material & Pu::Material::operator=(Material && other)
{
	if (this != &other)
	{
		UniformBlock::operator=(std::move(other));
		diffuseMap = other.diffuseMap;
		roughness = other.roughness;
		f0 = other.f0;
		diffuse = other.diffuse;

		other.diffuseMap = nullptr;
	}

	return *this;
}

/* Diffuse hides class member. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Material::SetParameters(float glossiness, Vector3 specular, Vector3 diffuse)
{
	roughness = 1.0f - glossiness;
	f0 = specular;
	this->diffuse = diffuse;
	IsDirty = true;
}

void Pu::Material::SetParameters(float glossiness, Color specular, Color diffuse)
{
	SetParameters(glossiness, specular.ToVector3(), diffuse.ToVector3());
}
#pragma warning(pop)

void Pu::Material::Stage(byte * dest)
{
	/* The values are alligned to 4 floats so we're wasting a bit of data here. */
	Copy(dest, &f0);
	Copy(dest + sizeof(Vector4), &diffuse);
	Copy(dest + (sizeof(Vector4) << 1), &roughness);
}