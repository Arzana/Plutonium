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
	poolMaterials(value.poolMaterials), poolProbes(value.poolProbes), textures(std::move(value.textures)),
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

void Pu::Model::AllocPools(const DeferredRenderer & deferred, const LightProbeRenderer * probes, size_t basicCount, size_t advancedCount)
{
	/* Reserve the material vectors to decrease allocations. */
	const uint32 totalCount = static_cast<uint32>(basicCount + advancedCount);
	materials.reserve(totalCount);
	probeMaterials.reserve(totalCount);

	/* Allocate the required descriptor pools. */
	poolMaterials = deferred.CreateMaterialDescriptorPool(static_cast<uint32>(basicCount), static_cast<uint32>(advancedCount));
	if (probes) poolProbes = probes->CreateDescriptorPool(static_cast<uint32>(totalCount));
}

/* This finalize is used to finalize normal models. */
void Pu::Model::Finalize(CommandBuffer & cmdBuffer, const DeferredRenderer & deferred, const LightProbeRenderer * probes, const PuMData & data)
{
	/* Calculate how many basic and advanced materials are needed. */
	size_t basicCount = 0, advancedCount = 0;
	for (const PumMaterial &raw : data.Materials)
	{
		basicCount += !raw.HasNormalTexture;
		advancedCount += raw.HasNormalTexture;
	}

	/* Allocate the pool and set the set values. */
	AllocPools(deferred, probes, basicCount, advancedCount);
	for (const PumMaterial &raw : data.Materials) AddMaterial(raw, deferred, probes);

	/* Only the material pool needs to update. */
	poolMaterials->Update(cmdBuffer, PipelineStageFlags::FragmentShader);
}

/* This finalize is used to finalize created models. */
void Pu::Model::Finalize(CommandBuffer & cmdBuffer, const DeferredRenderer & deferred, const LightProbeRenderer * probes)
{
	/* Allocate the single pool and set the material. */
	AllocPools(deferred, probes, 1, 0);	

	Material &material = AddBasicMaterial(deferred);
	material.SetDiffuse(*textures[0]);
	material.SetSpecular(*textures[1]);
	material.SetParameters(1.0f, 2.0f, Color::Black(), Color::White(), 1.0f);

	poolMaterials->Update(cmdBuffer, PipelineStageFlags::FragmentShader);
}

void Pu::Model::AddMaterial(const PumMaterial & info, const DeferredRenderer & deferred, const LightProbeRenderer * probes)
{
	/* Get the indices of the textures to use, the last 2 are default textures. */
	const size_t diffuse = info.HasDiffuseTexture ? info.DiffuseTexture : textures.size() - 2;
	const size_t specGlossMap = info.HasSpecGlossTexture ? info.SpecGlossTexture : textures.size() - 1;

	/* Create the light probe material for this material. */
	if (probes)
	{
		DescriptorSet &set = probeMaterials.emplace_back(*poolProbes, 0, probes->GetLayout());
		set.Write(probes->GetDiffuseDescriptor(), *textures[diffuse]);
	}

	/* Allocate the correct material and set the diffuse and specular map. */
	Material &material = info.HasNormalTexture ? AddAdvancedMaterial(info.NormalTexture, deferred) : AddBasicMaterial(deferred);
	material.SetDiffuse(*textures[diffuse]);
	material.SetSpecular(*textures[specGlossMap]);
	material.SetParameters(info);
}

Pu::Material & Pu::Model::AddAdvancedMaterial(size_t normal, const DeferredRenderer & deferred)
{
	/* Allocate the material from the pool. */
	Material &result = materials.emplace_back(*poolMaterials, DeferredRenderer::SubpassAdvancedStaticGeometry, deferred.GetAdvancedMaterialLayout());
	result.SetNormal(*textures[normal]);
	return result;
}

Pu::Material & Pu::Model::AddBasicMaterial(const DeferredRenderer & deferred)
{
	/* Allocate the material from the pool. */
	return materials.emplace_back(*poolMaterials, DeferredRenderer::SubpassBasicStaticGeometry, deferred.GetBasicMaterialLayout());
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