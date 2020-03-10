#include "Graphics/Models/Material.h"

Pu::Material::Material(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, 0, layout), diffuseMap(&GetDescriptor(0, "Diffuse")),
	specularMap(&GetDescriptor(0, "SpecularGlossiness")),
	normalMap(&GetDescriptor(0, "Bump")),
	emissiveMap(&GetDescriptor(0, "Emissive")),
	occlusionMap(&GetDescriptor(0, "Occlusion")),
	threshold(0.0f)
{}

Pu::Material::Material(DescriptorPool & pool, const DescriptorSetLayout & layout, const PumMaterial & parameters)
	: Material(pool, layout)
{
	SetParameters(parameters);
}

Pu::Material::Material(Material && value)
	: DescriptorSet(std::move(value)), diffuseMap(value.diffuseMap),  specular(value.specular),
	specularMap(value.specularMap), threshold(value.threshold), diffuse(value.diffuse),
	normalMap(value.normalMap), emissiveMap(value.emissiveMap), occlusionMap(value.occlusionMap)
{
	value.diffuseMap = nullptr;
	value.specularMap = nullptr;
	value.normalMap = nullptr;
	value.emissiveMap = nullptr;
	value.occlusionMap = nullptr;
}

Pu::Material & Pu::Material::operator=(Material && other)
{
	if (this != &other)
	{
		DescriptorSet::operator=(std::move(other));
		diffuseMap = other.diffuseMap;
		specularMap = other.specularMap;
		normalMap = other.normalMap;
		emissiveMap = other.emissiveMap;
		occlusionMap = other.occlusionMap;
		threshold = other.threshold;
		specular = other.specular;
		diffuse = other.diffuse;

		other.diffuseMap = nullptr;
		other.specularMap = nullptr;
		other.normalMap = nullptr;
		other.emissiveMap = nullptr;
		other.occlusionMap = nullptr;
	}

	return *this;
}

/* Diffuse hides class member. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Material::SetParameters(float glossiness, float specPower, Vector3 specular, Vector3 diffuse, float threshold)
{
	this->diffuse.W = 1.0f - glossiness;
	this->specular.W = specPower;
	this->specular.XYZ = specular;
	this->diffuse.XYZ = diffuse;
	this->threshold = threshold;
}

void Pu::Material::SetParameters(float glossiness, float specPower, Color specular, Color diffuse, float threshold)
{
	SetParameters(glossiness, specPower, specular.ToVector3(), diffuse.ToVector3(), threshold);
}
#pragma warning(pop)

void Pu::Material::Stage(byte * dest)
{
	Copy(dest, &specular);
	Copy(dest + sizeof(Vector4), &diffuse);
	Copy(dest + (sizeof(Vector4) << 1), &threshold);
}