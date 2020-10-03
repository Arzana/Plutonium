#include "Graphics/Models/Terrain.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::Terrain::Terrain(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassTerrain, layout),
	height(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Height")),
	mask(&GetDescriptor(DeferredRenderer::SubpassTerrain, "TextureMask")),
	textures(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Textures")),
	factors(0.1f, 1.0f, 20.0f, 1.0f), transform(0.0f, 0.0f, 0.0f, 1.0f)
{}

const Pu::Matrix & Pu::Terrain::GetTransform(void) const
{
	if (dirty)
	{
		dirty = false;
		mdl = Matrix::CreateScaledTranslation(transform.XYZ, transform.W);
	}

	return mdl;
}

void Pu::Terrain::SetPosition(Vector3 value)
{
	dirty = true;
	transform.XYZ = value;
}

void Pu::Terrain::SetScale(float value)
{
	dirty = true;
	transform.W = value;
}

void Pu::Terrain::Stage(byte * dest)
{
	Copy(dest, &GetTransform());
	Copy(dest + sizeof(Matrix), &factors);
}