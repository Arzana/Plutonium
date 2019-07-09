#include "Graphics/Models/ShapeCreator.h"

Pu::Mesh Pu::ShapeCreator::Plane(Buffer & src, const Buffer & dst)
{
	/* Begin the memory transfer operation. */
	if (CheckSrcBuffer(src, PlaneBufferSize)) return Mesh();
	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	/* Top Left. */
	vertices->Position = Vector3(-0.5f, 0.5f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	/* Bottom left. */
	vertices->Position = Vector3(-0.5f, -0.5f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	/* Bottom Right. */
	vertices->Position = Vector3(0.5f, -0.5f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	/* Top Right. */
	vertices->Position = Vector3(0.5f, 0.5f, 0.0f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	/* Indices. */
	uint16 *indices = reinterpret_cast<uint16*>(vertices);
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 2;
	indices[4] = 0;
	indices[5] = 3;

	src.EndMemoryTransfer();
	return Mesh(dst, 4 * sizeof(Basic3D), 6 * sizeof(uint16), sizeof(Basic3D), sizeof(uint16), IndexType::UInt16);
}

Pu::Mesh Pu::ShapeCreator::Box(Buffer & src, const Buffer & dst)
{
	/* Begin the memory transfer operation. */
	if (CheckSrcBuffer(src, BoxBufferSize)) return Mesh();
	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	/* Back face. */
	vertices->Position = Vector3(-0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Backward();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Backward();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Backward();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Backward();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	/* Front face. */
	vertices->Position = Vector3(-0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Forward();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	/* Left face. */
	vertices->Position = Vector3(-0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Left();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Left();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Left();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Left();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	/* Right face. */
	vertices->Position = Vector3(0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Right();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Right();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Right();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Right();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	/* Top face. */
	vertices->Position = Vector3(-0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Up();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Up();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, 0.5f);
	vertices->Normal = Vector3::Up();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, -0.5f, -0.5f);
	vertices->Normal = Vector3::Up();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	/* Bottom face. */
	vertices->Position = Vector3(-0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Down();
	vertices->TexCoord = Vector2(0.0f);
	++vertices;

	vertices->Position = Vector3(-0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Down();
	vertices->TexCoord = Vector2(0.0f, 1.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, 0.5f, 0.5f);
	vertices->Normal = Vector3::Down();
	vertices->TexCoord = Vector2(1.0f);
	++vertices;

	vertices->Position = Vector3(0.5f, 0.5f, -0.5f);
	vertices->Normal = Vector3::Down();
	vertices->TexCoord = Vector2(1.0f, 0.0f);
	++vertices;

	/* Indices. */
	uint16 *indices = reinterpret_cast<uint16*>(vertices);
	indices[0] = 0;
	indices[1] = 2;
	indices[2] = 1;
	indices[3] = 2;
	indices[4] = 0;
	indices[5] = 3;
	indices[6] = 4;
	indices[7] = 5;
	indices[8] = 6;
	indices[9] = 6;
	indices[10] = 7;
	indices[11] = 4;
	indices[12] = 8;
	indices[13] = 9;
	indices[14] = 10;
	indices[15] = 10;
	indices[16] = 11;
	indices[17] = 8;
	indices[18] = 12;
	indices[19] = 14;
	indices[20] = 13;
	indices[21] = 14;
	indices[22] = 12;
	indices[23] = 15;
	indices[24] = 16;
	indices[25] = 18;
	indices[26] = 17;
	indices[27] = 18;
	indices[28] = 16;
	indices[29] = 19;
	indices[30] = 20;
	indices[31] = 21;
	indices[32] = 22;
	indices[33] = 22;
	indices[34] = 23;
	indices[35] = 20;

	src.EndMemoryTransfer();
	return Mesh(dst, 24 * sizeof(Basic3D), 36 * sizeof(uint16), sizeof(Basic3D), sizeof(uint16), IndexType::UInt16);
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
