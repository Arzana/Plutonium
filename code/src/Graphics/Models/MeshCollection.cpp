#include "Graphics/Models/MeshCollection.h"

/* Checks (on debug) if initialize hasn't been called yet. */
#ifdef _DEBUG
#define DBG_INIT_CHECK(...)	if (memory) Log::Fatal("MeshCollection cannot be initialized twice!")
#else
#define DBG_INIT_CHECK(...)
#endif

Pu::MeshCollection::MeshCollection(void)
	: memory(nullptr)
{}

Pu::MeshCollection::MeshCollection(LogicalDevice & device, const PuMData & data)
	: memory(nullptr)
{
	Initialize(device, data);
}

Pu::MeshCollection::MeshCollection(MeshCollection && value)
	: memory(value.memory), views(std::move(value.views)),
	meshes(std::move(value.meshes)), boundingBox(value.boundingBox)
{
	value.memory = nullptr;
}

Pu::MeshCollection & Pu::MeshCollection::operator=(MeshCollection && other)
{
	if (this != &other)
	{
		Destroy();

		memory = other.memory;
		views = std::move(other.views);
		meshes = std::move(other.meshes);
		boundingBox = other.boundingBox;

		other.memory = nullptr;
	}

	return *this;
}

void Pu::MeshCollection::Initialize(LogicalDevice & device, const PuMData & data)
{
	DBG_INIT_CHECK();

	/* Allocate the GPU memory and set the views. */
	Alloc(device, *data.Buffer);
	views = data.Views;

	/* Add all the meshes to the list. */
	for (const PumMesh &geom : data.Geometry)
	{
		const uint32 matIdx = geom.HasMaterial ? geom.Material : DefaultMaterialIdx;
		meshes.emplace_back(std::make_pair(matIdx, Mesh{ geom }));
	}

	SetBoundingBox();
}

void Pu::MeshCollection::Initialize(LogicalDevice & device, StagingBuffer & src, uint32 vrtxSize, Mesh &&mesh)
{
	DBG_INIT_CHECK();

	/* Allocate the destination buffer and add the vertex and index view. */
	Alloc(device, src);
	views.emplace_back(0, vrtxSize);
	views.emplace_back(vrtxSize, src.GetSize() - vrtxSize);

	/* Add the mesh to the collection and set the bounding box. */
	meshes.emplace_back(std::make_pair(0u, std::move(mesh)));
	boundingBox = mesh.GetBoundingBox();
}

void Pu::MeshCollection::Alloc(LogicalDevice & device, const StagingBuffer & src)
{
	memory = new Buffer(device, src.GetSize(), BufferUsageFlag::IndexBuffer | BufferUsageFlag::VertexBuffer | BufferUsageFlag::TransferDst, false);
}

void Pu::MeshCollection::SetBoundingBox(void)
{
	if (meshes.size())
	{
		/* We cannot merge into from the start, because then [0, 0, 0] would always be included in the bounding box. */
		boundingBox = meshes.front().second.GetBoundingBox();
		for (size_t i = 1; i < meshes.size(); i++)
		{
			boundingBox.MergeInto(meshes[i].second.GetBoundingBox());
		}
	}
}

void Pu::MeshCollection::Destroy(void)
{
	if (memory) delete memory;
}