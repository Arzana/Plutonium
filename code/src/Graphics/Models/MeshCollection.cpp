#include "Graphics/Models/MeshCollection.h"

#ifdef _DEBUG
#define DBG_ADD_CHECK()		if (memory) Log::Fatal("Cannot add additional meshes to MeshCollection after it has been finalized!");
#else
#define DBG_ADD_CHECK()
#endif

Pu::MeshCollection::MeshCollection(void)
	: memory(nullptr)
{}

Pu::MeshCollection::MeshCollection(LogicalDevice & device, const PuMData & data)
	: memory(nullptr)
{
	AddMeshes(data, 0u);
	Finalize(device, data.Buffer->GetSize());
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

void Pu::MeshCollection::AddMesh(const Mesh & mesh)
{
	DBG_ADD_CHECK();
	meshes.emplace_back(std::make_pair(0u, mesh));
}

Pu::uint32 Pu::MeshCollection::AddView(size_t offset, size_t size)
{
	DBG_ADD_CHECK();
	views.emplace_back(offset, size);
	return static_cast<uint32>(views.size() - 1);
}

void Pu::MeshCollection::AddMesh(const Mesh & mesh, uint32 vertexStart, uint32 vertexSize, uint32 indexStart, uint32 indexSize)
{
	DBG_ADD_CHECK();

	views.emplace_back(vertexStart, vertexSize);
	views.emplace_back(indexStart, indexSize);
	meshes.emplace_back(std::make_pair(0u, mesh));
}

void Pu::MeshCollection::AddMesh(const Mesh & mesh, StagingBuffer & src, uint32 vertexStart, uint32 vertexSize)
{
	AddMesh(mesh, vertexStart, vertexSize, vertexSize, static_cast<uint32>(src.GetSize() - vertexSize));
}

void Pu::MeshCollection::AddMeshes(const PuMData & data, uint32 vertexStart)
{
	DBG_ADD_CHECK();

	/* Add the views to the buffer and add the specific offset. */
	for (PumView view : data.Views)
	{
		view.Offset += vertexStart;
		views.emplace_back(view);
	}

	/* Add all the meshes. */
	for (const PumMesh &geom : data.Geometry)
	{
		const uint32 mat = geom.HasMaterial ? geom.Material : DefaultMaterialIdx;
		meshes.emplace_back(std::make_pair(mat, Mesh{ geom }));
	}
}

void Pu::MeshCollection::Finalize(LogicalDevice & device, DeviceSize size)
{
#ifdef _DEBUG
	/* Calculate the final range of the views. */
	size_t begin = maxv<uint32>(), end = 0u;
	for (const PumView view : views)
	{
		begin = min(begin, view.Offset);
		end = max(end, view.Offset + view.Size);
	}

	/* Check for invalid usage. */
	if (end > size) Log::Fatal("Views in MeshCollection are larger than supplied buffer size!");
	if (begin) Log::Warning("MeshCollection is wasting memory (No mesh starts at 0)!");
	if (end < size) Log::Warning("MeshCollection is wasting memory (Unnecessary padding was added at the end)!");
#endif

	Alloc(device, size);
	SetBoundingBox();
}

void Pu::MeshCollection::Bind(CommandBuffer & cmdBuffer, uint32 binding, uint32 mesh) const
{
	/* 
	The full offset of the mesh into the buffer is its vertex view + the bind offset. 
	Normally we want to set the bind offset in the draw call to avoid needlessly binding buffers, 
	but this method is used if multiple vertex buffers need to be binded, 
	so we cannot save on vertex bind calls then.
	*/
	const Mesh &vrtx = meshes.at(mesh).second;
	cmdBuffer.BindVertexBuffer(binding, *memory, GetViewOffset(vrtx.GetVertexView()) + vrtx.GetBindOffset());
}

void Pu::MeshCollection::Alloc(LogicalDevice & device, size_t size)
{
	memory = new Buffer(device, size, BufferUsageFlags::IndexBuffer | BufferUsageFlags::VertexBuffer | BufferUsageFlags::TransferDst, MemoryPropertyFlags::None);
	memory->SetDebugName("MeshCollection");
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