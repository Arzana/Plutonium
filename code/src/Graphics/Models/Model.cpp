#include "Graphics/Models/Model.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/Lighting/LightProbeRenderer.h"

Pu::Model::Model(void)
	: Asset(true), Category(ModelCategory::Static), gpuData(nullptr),
	poolMaterials(nullptr), poolProbes(nullptr)
{}

Pu::Model::Model(Model && value)
	: Asset(std::move(value)), Category(value.Category), gpuData(value.gpuData),
	BasicMeshes(std::move(value.BasicMeshes)), AdvancedMeshes(std::move(value.AdvancedMeshes)),
	Materials(std::move(value.Materials)), ProbeMaterials(std::move(value.ProbeMaterials)),
	poolMaterials(value.poolMaterials), poolProbes(value.poolProbes), textures(value.textures)
{
	value.gpuData = nullptr;
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
		BasicMeshes = std::move(other.BasicMeshes);
		AdvancedMeshes = std::move(other.AdvancedMeshes);
		Materials = std::move(other.Materials);
		gpuData = other.gpuData;
		ProbeMaterials = std::move(other.ProbeMaterials);
		poolMaterials = other.poolMaterials;
		poolProbes = other.poolProbes;
		textures = std::move(other.textures);

		other.gpuData = nullptr;
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

void Pu::Model::AllocBuffer(LogicalDevice & device, const StagingBuffer & buffer)
{
	gpuData = new Buffer(device, buffer.GetSize(), BufferUsageFlag::IndexBuffer | BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst, false);
}

void Pu::Model::AllocPools(const DeferredRenderer & deferred, const LightProbeRenderer & probes, size_t count)
{
	/* Reserve the material vectors to decrease allocations. */
	Materials.reserve(count);
	ProbeMaterials.reserve(count);

	/* Allocate the required descriptor pools. */
	poolMaterials = deferred.CreateMaterialDescriptorPool(static_cast<uint32>(count));
	poolProbes = probes.CreateDescriptorPool(static_cast<uint32>(count));
}

void Pu::Model::Initialize(LogicalDevice & device, const PuMData & data)
{
	/* Allocate the GPU buffer. */
	AllocBuffer(device, *data.Buffer);
	nodes = data.Nodes;

	/* Load all the meshes. */
	for (const PumMesh &mesh : data.Geometry)
	{
		if (mesh.HasNormals && mesh.HasTextureCoordinates)
		{
			const uint32 matIdx = mesh.HasMaterial ? mesh.Material : DefaultMaterialIdx;
			if (mesh.HasTangents) AdvancedMeshes.emplace_back(std::make_pair(matIdx, Mesh{ *gpuData, data, mesh }));
			else BasicMeshes.emplace_back(std::make_pair(matIdx, Mesh{ *gpuData, data, mesh }));
		}
		else Log::Warning("Mesh '%ls' is not used because its vertex format is invalid!", mesh.Identifier.toWide().c_str());
	}

	/* Calculate the bounding box for all underlying meshes. */
	CalculateBoundingBox();

	/* Sort the meshes based on their materials. */
	const auto materialSort = [](const Shape &a, const Shape &b) { return a.first < b.first; };
	std::sort(BasicMeshes.begin(), BasicMeshes.end(), materialSort);
	std::sort(AdvancedMeshes.begin(), AdvancedMeshes.end(), materialSort);
}

void Pu::Model::Finalize(CommandBuffer & cmdBuffer, const DeferredRenderer & deferred, const LightProbeRenderer & probes, const PuMData & data)
{
	AllocPools(deferred, probes, data.Materials.size());

	for (const PumMaterial &raw : data.Materials)
	{
		/* Get the indices of the textures to use, the last 3 are default textures. */
		const size_t diffuseMap = raw.HasDiffuseTexture ? raw.DiffuseTexture : textures.size() - 2;
		const size_t specGlossMap = raw.HasSpecGlossTexture ? raw.SpecGlossTexture : textures.size() - 1;
		const size_t normalMap = raw.HasNormalTexture ? raw.NormalTexture : DefaultMaterialIdx;

		AddMaterial(diffuseMap, specGlossMap, normalMap, deferred, probes).SetParameters(raw);
	}

	/* Only the material pool needs to update. */
	poolMaterials->Update(cmdBuffer, PipelineStageFlag::FragmentShader);
}

Pu::Material& Pu::Model::AddMaterial(size_t diffuse, size_t specular, size_t normal, const DeferredRenderer & deferred, const LightProbeRenderer & probes)
{
	/* Create the light probe material for this material. */
	DescriptorSet &set = ProbeMaterials.emplace_back(*poolProbes, 0, probes.GetLayout());
	set.Write(probes.GetDiffuseDescriptor(), *textures[diffuse]);

	/* Create the deferred material for this material. */
	Material &material = Materials.emplace_back(*poolMaterials, deferred.GetMaterialLayout());
	material.SetDiffuse(*textures[diffuse]);
	material.SetSpecular(*textures[specular]);
	if (normal != DefaultMaterialIdx) material.SetNormal(*textures[normal]);

	return material;
}

void Pu::Model::CalculateBoundingBox(void)
{
	for (const auto[mat, mesh] : BasicMeshes)
	{
		boundingBox = boundingBox.Merge(mesh.GetBoundingBox());
	}

	for (const auto[mat, mesh] : AdvancedMeshes)
	{
		boundingBox = boundingBox.Merge(mesh.GetBoundingBox());
	}
}

void Pu::Model::Destroy(void)
{
	if (gpuData) delete gpuData;

	/* Delete the material descriptors in one go, and the delete their parent pool. */
	if (poolMaterials)
	{
		for (Material &cur : Materials) cur.Free();
		delete poolMaterials;
	}

	/* Delete the material descriptors in one go, and the delete their parent pool. */
	if (poolProbes)
	{
		for (DescriptorSet &cur : ProbeMaterials) cur.Free();
		delete poolProbes;
	}
}