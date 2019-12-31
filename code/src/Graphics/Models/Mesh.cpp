#include "Graphics/Models/Mesh.h"

Pu::Mesh::Mesh(void)
	: vertex(nullptr), index(nullptr)
{}

Pu::Mesh::Mesh(const Buffer & buffer, size_t vertexOffset, size_t vertexSize, size_t indexOffset, size_t indexSize, size_t vertexStride, size_t indexStride, IndexType indexType)
	: type(indexType), useIndexBuffer(true)
{
	vertex = new BufferView(buffer, vertexOffset, vertexSize, vertexStride);
	index = new BufferView(buffer, indexOffset, indexSize, indexStride);
}

Pu::Mesh::Mesh(const Buffer & buffer, size_t vertexSize, size_t indexSize, size_t vertexStride, size_t indexStride, IndexType indexType)
	: Mesh(buffer, 0, vertexSize, vertexSize, indexSize, vertexStride, indexStride, indexType)
{}

Pu::Mesh::Mesh(const Buffer & buffer, const PumMesh & mesh)
	: type(static_cast<IndexType>(mesh.IndexType)), useIndexBuffer(mesh.IndexType != PumIndexType::None),
	boundingBox(mesh.Bounds)
{
	/* Calculate the stride of the vertices. */
	size_t vertexStride = sizeof(Vector3);
	vertexStride += mesh.HasNormals * sizeof(Vector3);
	vertexStride += mesh.HasTangents * sizeof(Vector4);
	vertexStride += mesh.HasTextureCoordinates * sizeof(Vector2);
	vertexStride += mesh.HasColors * sizeof(uint32);
	if (mesh.JointType == PumJointType::Byte) vertexStride += sizeof(uint8) + sizeof(Vector4);
	else if (mesh.JointType == PumJointType::UShort) vertexStride += sizeof(uint16) + sizeof(Vector4);

	/* Create the buffer views. */
	vertex = new BufferView(buffer, mesh.VertexViewStart, mesh.VertexViewSize, vertexStride);
	if (useIndexBuffer)
	{
		const size_t indexStride = type == IndexType::UInt16 ? sizeof(uint16) : sizeof(uint32);
		index = new BufferView(buffer, mesh.IndexViewStart, mesh.IndexViewSize, indexStride);
	}
	else index = nullptr;
}

Pu::Mesh::Mesh(const Mesh & value)
	: type(value.type), useIndexBuffer(value.useIndexBuffer), boundingBox(value.boundingBox)
{
	vertex = new BufferView(*value.vertex);
	index = useIndexBuffer ? new BufferView(*value.index) : nullptr;
}

Pu::Mesh::Mesh(Mesh && value)
	: type(value.type), vertex(value.vertex),
	index(value.index), useIndexBuffer(value.useIndexBuffer),
	boundingBox(value.boundingBox)
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
		useIndexBuffer = other.useIndexBuffer;
		vertex = new BufferView(*other.vertex);
		index = useIndexBuffer ? new BufferView(*other.index) : nullptr;
		boundingBox = other.boundingBox;
	}

	return *this;
}

Pu::Mesh & Pu::Mesh::operator=(Mesh && other)
{
	if (this != &other)
	{
		Destroy();

		type = other.type;
		useIndexBuffer = other.useIndexBuffer;
		vertex = other.vertex;
		index = other.index;
		boundingBox = other.boundingBox;

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

	if (useIndexBuffer) cmdBuffer.BindIndexBuffer(*index, type);
	cmdBuffer.BindVertexBuffer(binding, *vertex);
}

void Pu::Mesh::DrawInstanced(CommandBuffer & cmdBuffer, uint32 instanceCount) const
{
	/* Only do this check on debug, should hardly occur. */
#ifdef _DEBUG
	Check();
#endif

	if (useIndexBuffer) cmdBuffer.Draw(static_cast<uint32>(index->GetElementCount()), instanceCount, 0, 0, 0);
	else cmdBuffer.Draw(static_cast<uint32>(vertex->GetElementCount()), instanceCount, 0, 0);
}

void Pu::Mesh::Check(void) const
{
	if (!vertex || (useIndexBuffer && !index)) Log::Fatal("Cannot bind invalid mesh!");
}

void Pu::Mesh::Destroy(void)
{
	if (vertex) delete vertex;
	if (index) delete index;
}