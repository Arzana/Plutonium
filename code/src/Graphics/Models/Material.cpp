#include "Graphics/Models/Material.h"

Pu::Material::Material(DescriptorPool & pool)
	: UniformBlock(pool, false), diffuseMap(&pool.GetSubpass().GetDescriptor("Diffuse")),
	specularMap(&pool.GetSubpass().GetDescriptor("SpecularGlossiness")),
	normalMap(&pool.GetSubpass().GetDescriptor("Normal")),
	roughness(1.0f), power(2.0f)
{}

Pu::Material::Material(DescriptorPool & pool, const PumMaterial & parameters)
	: Material(pool)
{
	SetParameters(parameters);
}

Pu::Material::Material(Material && value)
	: UniformBlock(std::move(value)), diffuseMap(value.diffuseMap), roughness(value.roughness), 
	specularMap(value.specularMap), power(value.power), f0(value.f0), diffuse(value.diffuse),
	normalMap(value.normalMap)
{
	value.diffuseMap = nullptr;
	value.specularMap = nullptr;
	value.normalMap = nullptr;
}

Pu::Material & Pu::Material::operator=(Material && other)
{
	if (this != &other)
	{
		UniformBlock::operator=(std::move(other));
		diffuseMap = other.diffuseMap;
		specularMap = other.specularMap;
		normalMap = other.normalMap;
		roughness = other.roughness;
		power = other.power;
		f0 = other.f0;
		diffuse = other.diffuse;

		other.diffuseMap = nullptr;
		other.specularMap = nullptr;
		other.normalMap = nullptr;
	}

	return *this;
}

/* Diffuse hides class member. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::Material::SetParameters(float glossiness, float specPower, Vector3 specular, Vector3 diffuse)
{
	roughness = 1.0f - glossiness;
	power = specPower;
	f0 = specular;
	this->diffuse = diffuse;
	IsDirty = true;
}

void Pu::Material::SetParameters(float glossiness, float specPower, Color specular, Color diffuse)
{
	SetParameters(glossiness, specPower, specular.ToVector3(), diffuse.ToVector3());
}
#pragma warning(pop)

void Pu::Material::Stage(byte * dest)
{
	constexpr size_t offset1 = sizeof(Vector4);
	constexpr size_t offset2 = offset1 + sizeof(Vector3);
	constexpr size_t offset3 = offset2 + sizeof(float);

	Copy(dest, &f0);					// Allignment 0.
	Copy(dest + offset1, &diffuse);		// Allignment 16, vec3 starts at 16 byte boundry
	Copy(dest + offset2, &roughness);	// Allignment 28, float starts at 4 byte boundry
	Copy(dest + offset3, &power);		// Allignment 32, float starts at 4 byte boundry
}