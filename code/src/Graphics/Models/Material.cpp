#include "Graphics/Models/Material.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::Material::Material(DescriptorPool & pool, uint32 subpass, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, subpass, layout), 
	diffuseMap(&GetDescriptor(subpass, "Diffuse")),
	specularMap(&GetDescriptor(subpass, "SpecularGlossiness")),
	normalMap(nullptr), threshold(0.0f)
{
	if (subpass == DeferredRenderer::SubpassAdvancedStaticGeometry)
	{
		normalMap = &GetDescriptor(subpass, "Bump");
	}
}

Pu::Material::Material(DescriptorPool & pool, uint32 subpass, const DescriptorSetLayout & layout, const PumMaterial & parameters)
	: Material(pool, subpass, layout)
{
	SetParameters(parameters);
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