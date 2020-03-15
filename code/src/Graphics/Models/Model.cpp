#include "Graphics/Models/Model.h"
#include "Graphics/Lighting/DeferredRenderer.h"
#include "Graphics/Lighting/LightProbeRenderer.h"

Pu::Model::Model(void)
	: Asset(true), Category(ModelCategory::Static), gpuData(nullptr),
	poolMaterials(nullptr), poolProbes(nullptr)
{}

Pu::Model::Model(Model && value)
	: Asset(std::move(value)), Category(value.Category), 
	Meshes(std::move(value.Meshes)), Materials(std::move(value.Materials)),
	gpuData(value.gpuData), ProbeMaterials(std::move(value.ProbeMaterials)),
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
		Meshes = std::move(other.Meshes);
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

void Pu::Model::Initialize(LogicalDevice & device, const PuMData & data)
{
	/* Allocate the GPU buffer. */
	gpuData = new Buffer(device, data.Buffer->GetSize(), BufferUsageFlag::IndexBuffer | BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst, false);

	/* Load all the meshes. */
	Meshes.reserve(data.Geometry.size());
	for (const PumMesh &mesh : data.Geometry)
	{
		if (mesh.HasNormals && mesh.HasTangents && mesh.HasTextureCoordinates)
		{
			const uint32 matIdx = mesh.HasMaterial ? mesh.Material : DefaultMaterialIdx;
			Meshes.emplace_back(std::make_pair(matIdx, Mesh{ *gpuData, mesh }));
		}
		else Log::Warning("Mesh '%ls' is not used because its vertex format is invalid!", mesh.Identifier.c_str());
	}

	/* Sort the model on material index to avoid material switches. */
	std::sort(Meshes.begin(), Meshes.end(), [](const Shape &a, const Shape &b) { return a.first < b.first; });
}

void Pu::Model::Finalize(CommandBuffer & cmdBuffer, const DeferredRenderer & deferred, const LightProbeRenderer & probes, const PuMData & data)
{
	/* Reserve the material vectors to decrease allocations. */
	Materials.reserve(data.Materials.size());
	ProbeMaterials.reserve(data.Materials.size());

	/* Allocate the required descriptor pools. */
	poolMaterials = deferred.CreateMaterialDescriptorPool(static_cast<uint32>(data.Materials.size()));
	poolProbes = probes.CreateDescriptorPool(static_cast<uint32>(data.Materials.size()));

	for (const PumMaterial &raw : data.Materials)
	{
		/* Get the indices of the textures to use, the last 3 are default textures. */
		const size_t diffuseMap = raw.HasDiffuseTexture ? raw.DiffuseTexture : textures.size() - 3;
		const size_t specGlossMap = raw.HasSpecGlossTexture ? raw.SpecGlossTexture : textures.size() - 2;
		const size_t normalMap = raw.HasNormalTexture ? raw.NormalTexture : textures.size() - 1;

		/* Create the light probe material for this material. */
		DescriptorSet &set = ProbeMaterials.emplace_back(*poolProbes, 0, probes.GetLayout());
		set.Write(probes.GetDiffuseDescriptor(), *textures[diffuseMap]);

		/* Create the deferred material for this material. */
		Material &material = Materials.emplace_back(*poolMaterials, deferred.GetMaterialLayout(), raw);
		material.SetDiffuse(*textures[diffuseMap]);
		material.SetSpecular(*textures[specGlossMap]);
		material.SetNormal(*textures[normalMap]);
	}

	/* Only the material pool needs to update. */
	poolMaterials->Update(cmdBuffer, PipelineStageFlag::FragmentShader);
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