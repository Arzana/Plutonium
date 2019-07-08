#include "Graphics/Models/ShapeCreator.h"
#include "Graphics/VertexLayouts/Basic3D.h"

size_t Pu::ShapeCreator::GetPlaneBufferSize(void)
{
	/*
	The plane has 4 vertices, that each have a position, normal and texture UV.
	The indices are uint16 and we have 6 of those.
	*/
	return 4 * sizeof(Basic3D) + 6 * sizeof(uint16);
}

Pu::Mesh Pu::ShapeCreator::Plane(Buffer & src, Buffer & dst)
{
	/* Begin the memory transfer operation. */
	if (CheckSrcBuffer(src, GetPlaneBufferSize())) return Mesh();
	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	/* Top Left. */
	vertices->Position = Vector3(-1.0f, 1.0f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	/* Bottom left. */
	vertices->Position = Vector3(-1.0f, -1.0f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	/* Bottom Right. */
	vertices->Position = Vector3(1.0f, -1.0f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	/* Top Right. */
	vertices->Position = Vector3(1.0f, 1.0f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	/* Indices. */
	uint16 *indices = reinterpret_cast<uint16*>(vertices);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 3;
	indices[5] = 0;

	src.EndMemoryTransfer();
	return Mesh(dst, 0, 4 * sizeof(Basic3D), 4 * sizeof(Basic3D), 6 * sizeof(uint16), sizeof(Basic3D), sizeof(uint16), IndexType::UInt16);
}

bool Pu::ShapeCreator::CheckSrcBuffer(Buffer & buffer, size_t requiredSize)
{
	/* Only check for if the input buffer is correct on debug mode. */
#ifdef _DEBUG
	if (buffer.GetSize() < requiredSize)
	{
		Log::Error("Buffer is not large enough to accommodate a plane!");
		return true;
	}

	if (!buffer.IsHostAccessible())
	{
		Log::Error("Source buffer for plane is not has accessible!");
		return true;
	}
#endif

	return false;
}
