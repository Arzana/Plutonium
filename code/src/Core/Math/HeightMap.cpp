#include "Core/Math/HeightMap.h"
#include "Core/Math/Interpolation.h"
#include "Core/Diagnostics/Logging.h"

Pu::HeightMap::HeightMap(size_t dimensions, float scale)
	: HeightMap(dimensions, dimensions, scale)
{}

Pu::HeightMap::HeightMap(size_t width, size_t height, float scale)
	: width(width), height(height), boundX(width - 1), boundY(height - 1)
{
	const Vector2 size{ static_cast<float>(boundX), static_cast<float>(boundY) };
	patchSize = (size * scale) / size;
	iPatchSize = recip(patchSize);

	Alloc();
}

Pu::HeightMap::HeightMap(const HeightMap & value)
	: width(value.width), height(value.height), patchSize(value.patchSize),
	boundX(value.boundX), boundY(value.boundY), iPatchSize(value.iPatchSize)
{
	Alloc();
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

void Pu::HeightMap::SetHeight(size_t x, size_t y, float value)
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot set height at [%zu, %zu] in heightmap (out of [%zu, %zu] range)!", x, y, width, height);
	}
#endif

	data[y * width + x] = value;
}

void Pu::HeightMap::SetHeight(size_t i, float value)
{
#ifdef _DEBUG
	if (i >= width * height)
	{
		Log::Fatal("Cannot set height at %zu in heightmap (out of range)!", i);
	}
#endif

	data[i] = value;
}

void Pu::HeightMap::CalculateNormals(float displacement)
{
	/* Allocate the normals if needed. */
	if (!normals) normals = reinterpret_cast<Vector3*>(malloc(width * height * sizeof(Vector3)));

	/* Loop through every location of the heightmap. */
	for (int32 y = 0; y < height; y++)
	{
		for (int32 x = 0; x < width; x++)
		{
			/* Construct a sobel filter matrix. */
			float sobel[3][3];
			for (int32 sy = 0; sy < 3; sy++)
			{
				for (int32 sx = 0; sx < 3; sx++)
				{
					/* Sample height with a clamp operation. */
					const size_t sampleX = clamp(x + (sx - 1), 0, static_cast<int32>(boundX));
					const size_t sampleY = clamp(y + (sy - 1), 0, static_cast<int32>(boundY));
					sobel[sy][sx] = GetHeight(sampleX, sampleY) * displacement;
				}
			}

			/* Construct the surface normal from the sobel samples. */
			const float nx = sobel[0][0] - sobel[2][0] + 2.0f * sobel[0][1] - 2.0f * sobel[2][1] + sobel[0][2] - sobel[2][2];
			const float nz = sobel[0][0] + 2.0f * sobel[1][0] + sobel[2][0] - sobel[0][2] - 2.0f * sobel[1][2] - sobel[2][2];
			const float ny = 0.25f * sqrtf(1.0f - sqr(nx) - sqr(nz));
			normals[y * width + x] = normalize(Vector3{ nx, ny, nz });
		}
	}
}

bool Pu::HeightMap::Contains(size_t x, size_t y) const
{
	return x < width && y < height;
}

bool Pu::HeightMap::Contains(Vector2 pos) const
{
	return ipart(pos.X * iPatchSize.X) < boundX && ipart(pos.Y * iPatchSize.Y) < boundY && pos.X > 0.0f && pos.Y > 0.0f;
}

float Pu::HeightMap::GetHeight(size_t x, size_t y) const
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot get height at [%zu, %zu] in heightmap (out of [%zu, %zu] range)!", x, y, width, height);
	}
#endif

	return data[y * width + x];
}

float Pu::HeightMap::GetHeight(Vector2 pos) const
{
	/* Convert the location into the patch position and the patch uv. */
	size_t px, py;
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

Pu::Vector3 Pu::HeightMap::GetNormal(size_t x, size_t y) const
{
#ifdef _DEBUG
	if (!Contains(x, y))
	{
		Log::Fatal("Cannot get normal at [%zu, %zu] in heightmap (out of [%zu, %zu] range)!", x, y, width, height);
	}
#endif

	return normals[y * width + x];
}

Pu::Vector3 Pu::HeightMap::GetNormal(Vector2 pos) const
{
	/* Convert the location into the patch position and the patch uv. */
	size_t px, py;
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
	size_t px, py;
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

bool Pu::HeightMap::TryGetHeight(size_t x, size_t y, float & output) const
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

void Pu::HeightMap::TransformPosition(Vector2 input, size_t & px, size_t & py, float & x, float & y) const
{
	/*
	Get the patch location from the terrain position.
	Also clamp this position to the furthest bound in order to not sample outside of the map.
	*/
	px = min(static_cast<size_t>(ipart(input.X * iPatchSize.X)), boundX - 1);
	py = min(static_cast<size_t>(ipart(input.Y * iPatchSize.Y)), boundY - 1);

	/* Get the relative position on the patch. */
	x = fmodf(input.X, patchSize.X) * iPatchSize.X;
	y = fmodf(input.Y, patchSize.Y) * iPatchSize.Y;
}

float Pu::HeightMap::QueryHeight(size_t apx, size_t apy, size_t bpx, size_t bpy, size_t cpx, size_t cpy, float t, float s) const
{
	return barycentric(GetHeight(apx, apy), GetHeight(bpx, bpy), GetHeight(cpx, cpy), t, s);
}

Pu::Vector3 Pu::HeightMap::QueryNormal(size_t apx, size_t apy, size_t bpx, size_t bpy, size_t cpx, size_t cpy, float t, float s) const
{
	return barycentric(GetNormal(apx, apy), GetNormal(bpx, bpy), GetNormal(cpx, cpy), t, s);
}

void Pu::HeightMap::Alloc(void)
{
	data = reinterpret_cast<float*>(malloc(width * height * sizeof(float)));
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