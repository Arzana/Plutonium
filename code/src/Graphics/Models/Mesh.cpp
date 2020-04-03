#include "Graphics/Models/Mesh.h"

Pu::Mesh::Mesh(void)
	: stride(0), vertexView(DefaultViewIdx), offset(0),
	indexView(DefaultViewIdx), first(0), count(0)
{}

Pu::Mesh::Mesh(uint32 indexCount, uint32 vertexView, uint32 indexView, uint32 firstIndex, size_t vertexStride, IndexType indexType)
	: type(indexType), stride(static_cast<uint32>(vertexStride)), vertexView(vertexView), 
	count(indexCount), indexView(indexView), first(firstIndex), offset(0)
{}

Pu::Mesh::Mesh(uint32 elementCount, size_t vertexStride, IndexType indexType)
	: Mesh(elementCount, 0, 1, 0, vertexStride, indexType)
{}

Pu::Mesh::Mesh(const PumMesh & mesh)
	: type(static_cast<IndexType>(mesh.IndexType)), boundingBox(mesh.Bounds), 
	stride(mesh.GetStride()), vertexView(mesh.VertexView)
{
	if (mesh.IndexType != PumIndexType::None)
	{
		const uint32 idxStride = mesh.GetIndexStride();

		indexView = mesh.IndexView;
		offset = static_cast<int32>(mesh.VertexViewStart) / stride;
		first = static_cast<uint32>(mesh.IndexViewStart) / idxStride;
		count = static_cast<uint32>(mesh.IndexViewSize) / idxStride;
	}
	else
	{
		indexView = DefaultViewIdx;
		offset = 0;
		first = static_cast<uint32>(mesh.VertexViewStart) / stride;
		count = static_cast<uint32>(mesh.VertexViewSize) / stride;
	}
}

void Pu::Mesh::Draw(CommandBuffer & cmdBuffer, uint32 instanceCount) const
{
	if (indexView != DefaultViewIdx) cmdBuffer.Draw(count, instanceCount, first, 0, offset);
	else cmdBuffer.Draw(count, instanceCount, first, 0);
}