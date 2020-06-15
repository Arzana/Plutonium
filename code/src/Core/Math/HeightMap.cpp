#include "Core/Math/HeightMap.h"
#include "Core/Math/Interpolation.h"
#include "Core/Diagnostics/Logging.h"
#include "Graphics/Diagnostics/DebugRenderer.h"

Pu::HeightMap::HeightMap(uint32 dimensions, float scale, bool addNormals)
	: HeightMap(dimensions, dimensions, scale, addNormals)
{}

Pu::HeightMap::HeightMap(uint32 width, uint32 height, float scale, bool addNormals)
	: width(width), height(height), boundX(width - 1), boundY(height - 1)
{
	const Vector2 size{ static_cast<float>(boundX), static_cast<float>(boundY) };
	patchSize = (size * scale) / size;
	iPatchSize = recip(patchSize);

	Alloc(addNormals);
}

Pu::HeightMap::HeightMap(const HeightMap & value)
	: width(value.width), height(value.height), patchSize(value.patchSize),
	boundX(value.boundX), boundY(value.boundY), iPatchSize(value.iPatchSize)
{
	Alloc(value.normals);
	Copy(value);
}

Pu::HeightMap::HeightMap(HeightMap && value)
	: width(value.width), height(value.height),
	patchSize(value.patchSize), iPatchSize(value.patchSize),
	boundX(value.boundX), boundY(value.boundY),
	data(value.data), normals(value.normals)
{
	value.data = nullptr;
	value.normals = nullptr;
}

Pu::HeightMap & Pu::HeightMap::operator=(const HeightMap & other)
{
	if (this != &other)
	{
		width = other.width;
		height = other.height;
		boundX = other.boundX;
		boundY = other.boundY;
		patchSize = other.patchSize;
		iPatchSize = other.iPatchSize;

		data = reinterpret_cast<float*>(realloc(data, width * height * sizeof(float)));
		if (other.normals) normals = reinterpret_cast<Vector3*>(realloc(normals, width * height * sizeof(Vector3)));
		Copy(other);
	}

	return *this;
}

Pu::HeightMap & Pu::HeightMap::operator=(HeightMap && other)
{
	if (this != &other)
	{
		width = other.width;
		height = other.height;
		boundX = other.boundX;
		boundY = other.boundY;
		patchSize = other.patchSize;
		iPatchSize = other.iPatchSize;
		data = other.data;
		normals = other.normals;

		other.data = nullptr;
		other.normals = nullptr;
	}

	return *this;
}

void Pu::HeightMap::SetHeight(uint32 x, uint32 y, float value)
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot set height at [%u, %u] in heightmap (out of [%u, %u] range)!", x, y, width, height);
	}
#endif

	data[y * width + x] = value;
}

void Pu::HeightMap::SetHeight(uint32 i, float value)
{
#ifdef _DEBUG
	if (i >= width * height)
	{
		Log::Fatal("Cannot set height at %zu in heightmap (out of range)!", i);
	}
#endif

	data[i] = value;
}

void Pu::HeightMap::SetNormal(uint32 x, uint32 y, Vector3 normal)
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot set normal at [%u, %u] in heightmap (out of [%u, %u] range)!", x, y, width, height);
	}

	if (!normals) Log::Fatal("Heightmap needs to be created with pre-allocated normals when calling SetNormal!");
#endif

	normals[y * width + x] = normal;
}

/* Height hides class member, checked and works as expected. */
#pragma warning(push)
#pragma warning(disable:4458)
void Pu::HeightMap::SetHeightAndNormal(uint32 x, uint32 y, float height, Vector3 normal)
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot set height and normal at [%u, %u] in heightmap (out of [%u, %u] range)!", x, y, width, this->height);
	}

	if (!normals) Log::Fatal("Heightmap needs to be created with pre-allocated normals when calling SetHeightAndNormal!");
#endif

	data[y * width + x] = height;
	normals[y * width + x] = normal;
}
#pragma warning(pop)

void Pu::HeightMap::CalculateNormals(float displacement)
{
#ifdef _DEBUG
	if (!normals) Log::Fatal("Heightmap needs to be allocated with normals in order to generate them!");
#endif

	/* Loop through every location of the heightmap. */
	for (uint32 y = 0; y < height; y++)
	{
		for (uint32 x = 0; x < width; x++)
		{
			/* Construct a sobel filter matrix. */
			float sobel[3][3];
			for (int32 sy = 0; sy < 3; sy++)
			{
				for (int32 sx = 0; sx < 3; sx++)
				{
					/* Sample height with a clamp operation. */
					const uint32 sampleX = clamp(x + (sx - 1), 0, static_cast<int32>(boundX));
					const uint32 sampleY = clamp(y + (sy - 1), 0, static_cast<int32>(boundY));
					sobel[sy][sx] = GetHeight(sampleX, sampleY) * displacement;
				}
			}

			/* Construct the surface normal from the sobel samples. */
			const float nx = sobel[0][0] - sobel[2][0] + 2.0f * sobel[0][1] - 2.0f * sobel[2][1] + sobel[0][2] - sobel[2][2];
			const float nz = sobel[0][0] + 2.0f * sobel[1][0] + sobel[2][0] - sobel[0][2] - 2.0f * sobel[1][2] - sobel[2][2];
			normals[y * width + x] = normalize(Vector3{ nx, 1.0f, nz });
		}
	}
}

bool Pu::HeightMap::Contains(uint32 x, uint32 y) const
{
	return x < width && y < height;
}

bool Pu::HeightMap::Contains(Vector2 pos) const
{
	return upart(pos.X * iPatchSize.X) < boundX && upart(pos.Y * iPatchSize.Y) < boundY && pos.X > 0.0f && pos.Y > 0.0f;
}

float Pu::HeightMap::GetHeight(uint32 x, uint32 y) const
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot get height at [%u, %u] in heightmap (out of [%u, %u] range)!", x, y, width, height);
	}
#endif

	return data[y * width + x];
}

float Pu::HeightMap::GetHeight(Vector2 pos) const
{
	/* Convert the location into the patch position and the patch uv. */
	uint32 px, py;
	float x, y;
	TransformPosition(pos, px, py, x, y);

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	if (x <= (1.0f - y))
	{
		/* Bottom-Left triangle. */
		const float t = -x - (y - 1.0f);
		return QueryHeight(px, py, px + 1, py, px, py + 1, t, x);
	}
	else
	{
		/* Top-Right triangle. */
		const float t = -(y - 1.0f);
		const float s = x + (y - 1.0f);
		return QueryHeight(px + 1, py, px + 1, py + 1, px, py + 1, t, s);
	}
}

Pu::Vector3 Pu::HeightMap::GetNormal(uint32 x, uint32 y) const
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot get normal at [%u, %u] in heightmap (out of [%u, %u] range)!", x, y, width, height);
	}
#endif

	return normals[y * width + x];
}

Pu::Vector3 Pu::HeightMap::GetNormal(Vector2 pos) const
{
	/* Convert the location into the patch position and the patch uv. */
	uint32 px, py;
	float x, y;
	TransformPosition(pos, px, py, x, y);

#ifdef _DEBUG
	if (!normals) Log::Fatal("Cannot access normal at [%f, %f] (normals have not been calculated)!", pos.X, pos.Y);
#endif

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	if (x <= (1.0f - y))
	{
		/* Bottom-Left triangle. */
		const float t = -x - (y - 1.0f);
		return QueryNormal(px, py, px + 1, py, px, py + 1, t, x);
	}
	else
	{
		/* Top-Right triangle. */
		const float t = -(y - 1.0f);
		const float s = x + (y - 1.0f);
		return QueryNormal(px + 1, py, px + 1, py + 1, px, py + 1, t, s);
	}
}

void Pu::HeightMap::GetHeightAndNormal(Vector2 pos, float & output, Vector3 & normal) const
{
	/* Convert the location into the patch position and the patch uv. */
	uint32 px, py;
	float x, y;
	TransformPosition(pos, px, py, x, y);

#ifdef _DEBUG
	if (!normals) Log::Fatal("Cannot access normal at [%f, %f] (normals have not been calculated)!", pos.X, pos.Y);
#endif

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	if (x <= (1.0f - y))
	{
		/* Bottom-Left triangle. */
		const float t = -x - (y - 1.0f);
		output = QueryHeight(px, py, px + 1, py, px, py + 1, t, x);
		normal = QueryNormal(px, py, px + 1, py, px, py + 1, t, x);
	}
	else
	{
		/* Top-Right triangle. */
		const float t = -(y - 1.0f);
		const float s = x + (y - 1.0f);
		output = QueryHeight(px + 1, py, px + 1, py + 1, px, py + 1, t, s);
		normal = QueryNormal(px + 1, py, px + 1, py + 1, px, py + 1, t, s);
	}
}

bool Pu::HeightMap::TryGetHeight(uint32 x, uint32 y, float & output) const
{
	if (Contains(x, y))
	{
		output = GetHeight(x, y);
		return true;
	}

	return false;
}

bool Pu::HeightMap::TryGetHeight(Vector2 pos, float & output) const
{
	if (Contains(pos))
	{
		output = GetHeight(pos);
		return true;
	}

	return false;
}

bool Pu::HeightMap::TryGetNormal(Vector2 pos, Vector3 & normal) const
{
	if (Contains(pos))
	{
		normal = GetNormal(pos);
		return true;
	}

	return false;
}

bool Pu::HeightMap::TryGetHeightAndNormal(Vector2 pos, float & output, Vector3 & normal) const
{
	if (Contains(pos))
	{
		GetHeightAndNormal(pos, output, normal);
		return true;
	}

	return false;
}

void Pu::HeightMap::Visualize(DebugRenderer & renderer, Vector3 offset, Color color) const
{
	Vector3 a;
	Vector3 b;

	for (uint32 y = 0; y < height; y++)
	{
		for (uint32 x = 0; x < width - 1; x++)
		{
			a = offset + Vector3{ x * patchSize.X, data[y * width + x], y * patchSize.Y };

			/* Only render a downwards line if this is not the last row. */
			if (y < height - 1)
			{
				b = offset + Vector3{ x * patchSize.X, data[(y + 1) * width + x], (y + 1) * patchSize.Y };
				renderer.AddLine(a, b, color);
			}

			b = offset + Vector3{ (x + 1) * patchSize.X, data[y * width + x + 1], y * patchSize.Y };
			renderer.AddLine(a, b, color);
		}

		/* Render the last column line seperately */
		a = offset + Vector3{ (width - 1) * patchSize.X, data[(y + 1) * width + width - 1], (y + 1) * patchSize.Y };
		renderer.AddLine(a, b, color);
	}
}

void Pu::HeightMap::TransformPosition(Vector2 input, uint32 & px, uint32 & py, float & x, float & y) const
{
	/*
	Get the patch location from the terrain position.
	Also clamp this position to the furthest bound in order to not sample outside of the map.
	*/
	px = min(upart(input.X * iPatchSize.X), boundX - 1);
	py = min(upart(input.Y * iPatchSize.Y), boundY - 1);

	/* Get the relative position on the patch. */
	x = fmodf(input.X, patchSize.X) * iPatchSize.X;
	y = fmodf(input.Y, patchSize.Y) * iPatchSize.Y;
}

float Pu::HeightMap::QueryHeight(uint32 apx, uint32 apy, uint32 bpx, uint32 bpy, uint32 cpx, uint32 cpy, float t, float s) const
{
	return barycentric(GetHeight(apx, apy), GetHeight(bpx, bpy), GetHeight(cpx, cpy), t, s);
}

Pu::Vector3 Pu::HeightMap::QueryNormal(uint32 apx, uint32 apy, uint32 bpx, uint32 bpy, uint32 cpx, uint32 cpy, float t, float s) const
{
	return barycentric(GetNormal(apx, apy), GetNormal(bpx, bpy), GetNormal(cpx, cpy), t, s);
}

void Pu::HeightMap::Alloc(bool allocNormals)
{
	data = reinterpret_cast<float*>(malloc(width * height * sizeof(float)));
	if (allocNormals) normals = reinterpret_cast<Vector3*>(malloc(width * height * sizeof(Vector3)));
}

void Pu::HeightMap::Copy(const HeightMap & other)
{
	memcpy(data, other.data, width * height * sizeof(float));
	if (other.normals) memcpy(normals, other.normals, width * height * sizeof(Vector3));
}

void Pu::HeightMap::Free(void)
{
	if (data) free(data);
	if (normals) free(normals);
}