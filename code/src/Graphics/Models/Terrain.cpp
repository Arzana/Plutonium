#include "Graphics/Models/Terrain.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::Terrain::Terrain(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassTerrain, layout),
	height(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Height")),
	mask(&GetDescriptor(DeferredRenderer::SubpassTerrain, "TextureMask")),
	textures(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Textures")),
	factors(0.1f, 1.0f, 20.0f, 1.0f)
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
	/*
	The patch size (used for culling) is set to 1.5 times the scale of the patch. 
	This seems to cull in the most efficient manner.
	*/
	dirty = true;
	transform.W = value;
	factors.W = 1.5f * value;
}

void Pu::Terrain::Stage(byte * dest)
{
	Copy(dest, &GetTransform());
	Copy(dest + sizeof(Matrix), &factors);
}