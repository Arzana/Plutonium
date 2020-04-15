#include "Graphics/Models/Terrain.h"
#include "Graphics/Lighting/DeferredRenderer.h"

Pu::Terrain::Terrain(DescriptorPool & pool, const DescriptorSetLayout & layout)
	: DescriptorSet(pool, DeferredRenderer::SubpassTerrain, layout),
	height(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Height")),
	mask(&GetDescriptor(DeferredRenderer::SubpassTerrain, "TextureMask")),
	textures(&GetDescriptor(DeferredRenderer::SubpassTerrain, "Textures")),
	factors(0.0f, 1.0f, 20.0f)
{}

void Pu::Terrain::Stage(byte * dest)
{
	Copy(dest, &mdl);
	Copy(dest + sizeof(Matrix), &factors);
}