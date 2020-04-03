#include "Graphics/Models/ShapeCreator.h"

#ifdef _DEBUG
#define DBG_CHECK_BUFFER_SIZE(size)	if (CheckSrcBuffer(src, size)) return Mesh()
#else
#define DBG_CHECK_BUFFER_SIZE(...)
#endif

Pu::uint32 Pu::ShapeCreator::GetSphereVertexSize(uint16 divisions)
{
	return 6 * sqr(divisions + 1) * sizeof(Basic3D);
}

Pu::uint32 Pu::ShapeCreator::GetDomeVertexSize(uint16 divisions)
{
	return sqr(divisions + 1) * sizeof(Basic3D);
}

Pu::uint32 Pu::ShapeCreator::GetTorusVertexSize(uint16 divisions)
{
	return sqr(divisions + 1) * sizeof(Basic3D);
}

size_t Pu::ShapeCreator::GetSphereBufferSize(uint16 divisions)
{
	/*
	for 6 faces
		for i divisions + 1
			for j divisions + 1
				1 vertex
				if (i != divisions && j != divisions) 6 indices
	*/
	return GetSphereVertexSize(divisions) + 36 * sqr(divisions) * sizeof(uint16);
}

size_t Pu::ShapeCreator::GetDomeBufferSize(uint16 divisions)
{
	/*
	north pole = 1 vertex

	for i in divisions
		for j in divisions
			1 vertex

	for i in divisions
		3 indices (north pole)

	for i in divisions
		3 indices (north pole)
		for j in divisions
			6 indices (face)
	*/
	return GetDomeVertexSize(divisions) + (3 * divisions + 6 * sqr(divisions - 1)) * sizeof(uint16);
}

size_t Pu::ShapeCreator::GetTorusBufferSize(uint16 divisions)
{
	/*
	for i divisions
		for j in divisions
			1 vertex
			6 indices (face)

	We will have 1 more division than indices so add one to divisions for vertex part.
	*/
	return GetTorusVertexSize(divisions) + sqr(divisions) * 6 * sizeof(uint16);
}

Pu::Mesh Pu::ShapeCreator::Plane(Buffer & src)
{
	DBG_CHECK_BUFFER_SIZE(PlaneBufferSize);

	/* Begin the memory transfer operation. */
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
	return Mesh(6, sizeof(Basic3D), IndexType::UInt16);
}

Pu::Mesh Pu::ShapeCreator::Box(Buffer & src)
{
	DBG_CHECK_BUFFER_SIZE(BoxBufferSize);

	/* Begin the memory transfer operation. */
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
	return Mesh(36, sizeof(Basic3D), IndexType::UInt16);
}

/* https://github.com/caosdoar/spheres */
Pu::Mesh Pu::ShapeCreator::Sphere(Buffer & src, uint16 divisions)
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

	DBG_CHECK_BUFFER_SIZE(GetSphereBufferSize(divisions));

	/* Begin the memory transfer operation. */
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

	src.EndMemoryTransfer();
	return Mesh(36 * sqr(divisions), sizeof(Basic3D), IndexType::UInt16);
}

Pu::Mesh Pu::ShapeCreator::Dome(Buffer & src, uint16 divisions)
{
	DBG_CHECK_BUFFER_SIZE(GetDomeBufferSize(divisions));

	/* Begin the memory transfer operation. */
	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	const float divs = static_cast<float>(divisions);
	const float step = recip(divs - 1.0f);

	/* Create the vertices for the hemisphere. */
	for (float parallel = 0; parallel < divs; parallel++)
	{
		const float uvx = parallel * step;
		const float theta = uvx * TAU;
		const float ct = cosf(theta);
		const float st = sinf(theta);

		for (float meridian = 0; meridian < divs; meridian++)
		{
			const float uvy = (meridian + 1.0f) * step;
			const float phi = ((1.0f - uvy) * 0.5f) * -PI;
			const float cp = cosf(phi);

			vertices->Position = Vector3(cp * ct, sinf(phi), cp * st);
			vertices->Normal = normalize(vertices->Position);
			vertices->TexCoord = Vector2(uvx, uvy);
			++vertices;
		}
	}

	/* Add the noth pole. */
	vertices->Position = Vector3::Up();
	vertices->Normal = Vector3::Up();
	vertices->TexCoord = Vector2(0.5f, 1.0f);

	/* Indices. */
	uint16 *indices = reinterpret_cast<uint16*>(++vertices);
	for (uint16 parallel = 0; parallel < divisions - 1; parallel++)
	{
		const uint16 aStart = parallel * divisions;
		const uint16 bStart = (parallel + 1) * divisions;
		for (uint16 meridian = 0; meridian < divisions - 1; meridian++, indices += 6)
		{
			const uint16 a = aStart + meridian;
			const uint16 b = aStart + meridian + 1;
			const uint16 c = bStart + meridian;
			const uint16 d = bStart + meridian + 1;

			indices[0] = a;
			indices[1] = b;
			indices[2] = d;
			indices[3] = a;
			indices[4] = d;
			indices[5] = c;
		}
	}

	/* North pole indices. */
	for (uint16 meridian = 0; meridian < divisions; meridian++, indices += 3)
	{
		indices[0] = static_cast<int16>(sqr(divisions));
		indices[1] = meridian * divisions;
		indices[2] = (meridian + 1) * divisions;
	}

	src.EndMemoryTransfer();
	return Mesh((3 * divisions + 6 * sqr(divisions - 1)), sizeof(Basic3D), IndexType::UInt16);
}

Pu::Mesh Pu::ShapeCreator::Torus(Buffer & src, uint16 divisions, float ratio)
{
	DBG_CHECK_BUFFER_SIZE(GetTorusBufferSize(divisions));

	/* Precast often used values. */
	const float divs = static_cast<float>(divisions);
	const float step = TAU / divs;
	constexpr float iTau = recip(TAU);

	src.BeginMemoryTransfer();
	Basic3D *vertices = reinterpret_cast<Basic3D*>(src.GetHostMemory());

	for (float theta = 0.0f; theta < TAU; theta += step)
	{
		const float ct = cosf(theta);
		const float st = sinf(theta);
		const float cr = ct * ratio;
		const float sr = st * ratio;
		const float u = theta * iTau;

		for (float phi = 0.0f; phi < TAU; phi += step, ++vertices)
		{
			const float cp = cosf(phi);
			const float sp = sinf(phi);

			const float x = ct + cp * cr;
			const float y = st + cp * sr;
			const float z = cp + sp * ratio;
			const float v = phi * iTau;

			vertices->Position = Vector3(x, y, z);
			vertices->Normal = normalize(Vector3(cp * ct, cp * st, st * ratio));
			vertices->TexCoord = Vector2(u, v);
		}
	}

	uint16 *indices = reinterpret_cast<uint16*>(vertices);
	const uint16 d2 = divisions + 1;
	for (uint16 i = 0; i < divisions; i++)
	{
		const uint16 i2 = i + 1;
		for (uint16 j = 0; j < divisions; j++)
		{
			const uint16 j2 = j + 1;

			indices[0] = i * d2 + j;
			indices[1] = i * d2 + j2;
			indices[2] = i2 * d2 + j;
			indices[3] = i2 * d2 + j2;
			indices[4] = i2 * d2 + j;
			indices[5] = i * d2 + j2;
		}
	}

	src.EndMemoryTransfer();
	return Mesh(6 * sqr(divisions), sizeof(Basic3D), IndexType::UInt16);
}

#ifdef _DEBUG
bool Pu::ShapeCreator::CheckSrcBuffer(Buffer & buffer, size_t requiredSize)
{
	if (buffer.GetSize() < requiredSize)
	{
		Log::Error("Buffer is not large enough to accommodate the primitive!");
		return true;
	}

	if (!buffer.IsHostAccessible())
	{
		Log::Error("Source buffer for plane is not has accessible!");
		return true;
	}

	return false;
}
#endif