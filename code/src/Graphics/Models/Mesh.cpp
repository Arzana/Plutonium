#include "Graphics/Models/Mesh.h"

Pu::Mesh::Mesh(void)
	: vertex(nullptr), index(nullptr)
{}

Pu::Mesh::Mesh(const Buffer & buffer, size_t vertexOffset, size_t vertexSize, size_t indexOffset, size_t indexSize, size_t vertexStride, size_t indexStride, IndexType indexType)
	: type(indexType)
{
	vertex = new BufferView(buffer, vertexOffset, vertexSize, vertexStride);
	index = new BufferView(buffer, indexOffset, indexSize, indexStride);
}

Pu::Mesh::Mesh(const Buffer & buffer, size_t vertexSize, size_t indexSize, size_t vertexStride, size_t indexStride, IndexType indexType)
	: Mesh(buffer, 0, vertexSize, vertexSize, indexSize, vertexStride, indexStride, indexType)
{}

Pu::Mesh::Mesh(const Buffer & buffer, const PumMesh & mesh)
	: type(static_cast<IndexType>(mesh.IndexType))
{
	/* Calculate the stride of the vertices. */
	size_t vertexStride = sizeof(Vector3);
	if (mesh.HasNormals) vertexStride += sizeof(Vector3);
	if (mesh.HasTangents) vertexStride += sizeof(Vector3);
	if (mesh.HasTextureCoordinates) vertexStride += sizeof(Vector2);
	if (mesh.HasColors) vertexStride += sizeof(uint32);

	/* Calculate the size of the indices. */
	const size_t indexStride = type == IndexType::UInt16 ? sizeof(uint16) : sizeof(uint32);

	/* Create the buffer views. */
	vertex = new BufferView(buffer, mesh.VertexViewStart, mesh.VertexViewSize, vertexStride);
	index = new BufferView(buffer, mesh.IndexViewStart, mesh.IndexViewSize, indexStride);
}

Pu::Mesh::Mesh(const Mesh & value)
	: type(value.type)
{
	vertex = new BufferView(*value.vertex);
	index = new BufferView(*value.index);
}

Pu::Mesh::Mesh(Mesh && value)
	: type(value.type), vertex(value.vertex), index(value.index)
{
	value.vertex = nullptr;
	value.index = nullptr;
}

Pu::Mesh & Pu::Mesh::operator=(const Mesh & other)
{
	if (this != &other)
	{
		Destroy();

		type = other.type;
		vertex = new BufferView(*other.vertex);
		index = new BufferView(*other.index);
	}

	return *this;
}

Pu::Mesh & Pu::Mesh::operator=(Mesh && other)
{
	if (this != &other)
	{
		Destroy();

		type = other.type;
		vertex = other.vertex;
		index = other.index;

		other.vertex = nullptr;
		other.index = nullptr;
	}

	return *this;
}

void Pu::Mesh::Bind(CommandBuffer & cmdBuffer, uint32 binding) const
{
	/* Only do this check on debug, should hardly occur. */
#ifdef _DEBUG
	Check();
#endif

	cmdBuffer.BindIndexBuffer(*index, type);
	cmdBuffer.BindVertexBuffer(binding, *vertex);
}

void Pu::Mesh::DrawInstanced(CommandBuffer & cmdBuffer, uint32 instanceCount) const
{
	/* Only do this check on debug, should hardly occur. */
#ifdef _DEBUG
	Check();
#endif

	cmdBuffer.Draw(static_cast<uint32>(index->GetElementCount()), instanceCount, 0, 0, 0);
}

void Pu::Mesh::Check(void) const
{
	if (!vertex || !index) Log::Fatal("Cannot bind invalid mesh!");
}

void Pu::Mesh::Destroy(void)
{
	if (vertex) delete vertex;
	if (index) delete index;
}