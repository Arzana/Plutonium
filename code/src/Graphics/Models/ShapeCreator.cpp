#include "Graphics/Models/ShapeCreator.h"

size_t Pu::ShapeCreator::GetSphereBufferSize(size_t divisions)
{
	/*
	for 6 faces
		for i divisions
			for j divisions
				1 vertex
				6 indices
	*/
	return 6 * sqr(divisions + 1) * (sizeof(Basic3D) + 6 * sizeof(uint16));
}

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

/* https://github.com/caosdoar/spheres */
Pu::Mesh Pu::ShapeCreator::Sphere(Buffer & src, const Buffer & dst, uint16 divisions)
{
	static const Vector3 origins[6] =
	{
		Vector3(-1.0, -1.0, -1.0),
		Vector3(1.0, -1.0, -1.0),
		Vector3(1.0, -1.0, 1.0),
		Vector3(-1.0, -1.0, 1.0),
		Vector3(-1.0, 1.0, -1.0),
		Vector3(-1.0, -1.0, 1.0)
	};

	static const Vector3 rights[6] =
	{
		Vector3(2.0, 0.0, 0.0),
		Vector3(0.0, 0.0, 2.0),
		Vector3(-2.0, 0.0, 0.0),
		Vector3(0.0, 0.0, -2.0),
		Vector3(2.0, 0.0, 0.0),
		Vector3(2.0, 0.0, 0.0)
	};

	static const Vector3 ups[6] =
	{
		Vector3(0.0, 2.0, 0.0),
		Vector3(0.0, 2.0, 0.0),
		Vector3(0.0, 2.0, 0.0),
		Vector3(0.0, 2.0, 0.0),
		Vector3(0.0, 0.0, 2.0),
		Vector3(0.0, 0.0, -2.0)
	};

	/* Begin the memory transfer operation. */
	if (CheckSrcBuffer(src, BoxBufferSize)) return Mesh();
	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	/* Loop through the faces of the box. */
	const float divs = static_cast<float>(divisions + 1);
	const float step = recip(divs - 1.0f);
	for (size_t i = 0; i < 6; i++)
	{
		const Vector3 origin = origins[i];
		const Vector3 right = rights[i];
		const Vector3 up = ups[i];

		/* Divide the box into subdivisions. */
		for (float x = 0; x < divs; x++)
		{
			for (float y = 0; y < divs; y++)
			{
				vertices->Position = origin + step * (x * right + y * up);
				const Vector3 p2 = sqr(vertices->Position);

				/* Set the position. */
				vertices->Position.X *= sqrtf(1.0f - 0.5f * (p2.Y + p2.Z) + p2.Y * p2.Z / 3.0f);
				vertices->Position.Y *= sqrtf(1.0f - 0.5f * (p2.Z + p2.X) + p2.Z * p2.X / 3.0f);
				vertices->Position.Z *= sqrtf(1.0f - 0.5f * (p2.X + p2.Y) + p2.X * p2.Y / 3.0f);

				/*
				The normal is simply the position normalized because we're centering this sphere around the origin.
				The texture coordinate is also very simple to calculate.
				*/
				vertices->Normal = normalize(vertices->Position);
				vertices->TexCoord = Vector2(x * step, y * step);
				++vertices;
			}
		}
	}

	/* Indices. */
	uint16 *indices = reinterpret_cast<uint16*>(vertices);
	for (uint16 i = 0, k = divisions + 1; i < 6; i++)
	{
		for (uint16 x = 0; x < divisions; x++)
		{
			const bool bottom = x < (divisions >> 1);
			for (uint16 y = 0; y < divisions; y++, indices += 6)
			{
				const bool left = y < (divisions >> 1);
				
				const uint16 a = (i * k + x) * k + y;
				const uint16 b = (i * k + x) * k + y + 1;
				const uint16 c = (i * k + x + 1) * k + y;
				const uint16 d = (i * k + x + 1) * k + y + 1;

				/* 
				The subdivision of the cubes looks better if we orient 
				the faces to the center of the cube.
				*/
				if (bottom ^ left)
				{
					indices[0] = a;
					indices[1] = b;
					indices[2] = c;
					indices[3] = c;
					indices[4] = b;
					indices[5] = d;
				}
				else
				{
					indices[0] = a;
					indices[1] = d;
					indices[2] = c;
					indices[3] = a;
					indices[4] = b;
					indices[5] = d;
				}
			}
		}
	}

	return Mesh(dst, 6 * sqr(divisions + 1) * sizeof(Basic3D), 36 * sqr(divisions) * sizeof(uint16), sizeof(Basic3D), sizeof(uint16), IndexType::UInt16);
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
