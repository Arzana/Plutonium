#include "Graphics/Models/Model.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/Lighting/LightProbeRenderer.h"
#include "Graphics/VertexLayouts/Advanced3D.h"
#include "Graphics/VertexLayouts/Basic3D.h"

Pu::Model::Model(void)
	: Asset(true), Category(ModelCategory::Static),
	poolMaterials(nullptr), poolProbes(nullptr)
{}

Pu::Model::Model(Model && value)
	: Asset(std::move(value)), Category(value.Category), meshes(std::move(value.meshes)),
	materials(std::move(value.materials)), probeMaterials(std::move(value.probeMaterials)),
	poolMaterials(value.poolMaterials), poolProbes(value.poolProbes), textures(value.textures),
	nodes(std::move(value.nodes))
{
	value.poolMaterials = nullptr;
	value.poolProbes = nullptr;
}

Pu::Model & Pu::Model::operator=(Model && other)
{
	if (this != &other)
	{
		Destroy();
		Asset::operator=(std::move(other));

		Category = other.Category;
		materials = std::move(other.materials);
		probeMaterials = std::move(other.probeMaterials);
		poolMaterials = other.poolMaterials;
		poolProbes = other.poolProbes;
		textures = std::move(other.textures);
		nodes = std::move(other.nodes);

		other.poolMaterials = nullptr;
		other.poolProbes = nullptr;
	}

	return *this;
}

Pu::Asset & Pu::Model::Duplicate(AssetCache&)
{
	Reference();
	for (Texture2D * cur : textures) cur->Reference();
	return *this;
}

void Pu::Model::AllocPools(const DeferredRenderer & deferred, const LightProbeRenderer * probes, size_t count)
{
	/* Reserve the material vectors to decrease allocations. */
	materials.reserve(count);
	probeMaterials.reserve(count);

	/* Allocate the required descriptor pools. */
	poolMaterials = deferred.CreateMaterialDescriptorPool(static_cast<uint32>(count));
	if (probes) poolProbes = probes->CreateDescriptorPool(static_cast<uint32>(count));
}

void Pu::Model::Finalize(CommandBuffer & cmdBuffer, const DeferredRenderer & deferred, const LightProbeRenderer * probes, const PuMData & data)
{
	AllocPools(deferred, probes, data.Materials.size());

	for (const PumMaterial &raw : data.Materials)
	{
		/* Get the indices of the textures to use, the last 3 are default textures. */
		const size_t diffuseMap = raw.HasDiffuseTexture ? raw.DiffuseTexture : textures.size() - 3;
		const size_t specGlossMap = raw.HasSpecGlossTexture ? raw.SpecGlossTexture : textures.size() - 2;
		const size_t normalMap = raw.HasNormalTexture ? raw.NormalTexture : textures.size() - 1;

		AddMaterial(diffuseMap, specGlossMap, normalMap, deferred, probes).SetParameters(raw);
	}

	/* Only the material pool needs to update. */
	poolMaterials->Update(cmdBuffer, PipelineStageFlag::FragmentShader);
}

Pu::Material& Pu::Model::AddMaterial(size_t diffuse, size_t specular, size_t normal, const DeferredRenderer & deferred, const LightProbeRenderer * probes)
{
	/* Create the light probe material for this material. */
	if (probes)
	{
		DescriptorSet &set = probeMaterials.emplace_back(*poolProbes, 0, probes->GetLayout());
		set.Write(probes->GetDiffuseDescriptor(), *textures[diffuse]);
	}

	/* Create the deferred material for this material. */
	Material &material = materials.emplace_back(*poolMaterials, deferred.GetMaterialLayout());
	material.SetDiffuse(*textures[diffuse]);
	material.SetSpecular(*textures[specular]);
	material.SetNormal(*textures[normal]);

	return material;
}

void Pu::Model::Destroy(void)
{
	/* Delete the material descriptors in one go, and the delete their parent pool. */
	if (poolMaterials)
	{
		for (Material &cur : materials) cur.Free();
		delete poolMaterials;
	}

	/* Delete the material descriptors in one go, and the delete their parent pool. */
	if (poolProbes)
	{
		for (DescriptorSet &cur : probeMaterials) cur.Free();
		delete poolProbes;
	}
}