#include "Core/Math/HeightMap.h"
#include "Core/Math/Interpolation.h"
#include "Core/Diagnostics/Logging.h"

Pu::HeightMap::HeightMap(size_t dimensions, float scale)
	: HeightMap(dimensions, dimensions, scale)
{}

Pu::HeightMap::HeightMap(size_t width, size_t height, float scale)
	: width(width), height(height), scale(scale), iscale(recip(scale)),
	boundX(width - 1), boundY(height - 1)
{
	Alloc();
}

Pu::HeightMap::HeightMap(const HeightMap & value)
	: width(value.width), height(value.height), scale(value.scale),
	boundX(value.boundX), boundY(value.boundY), iscale(value.iscale)
{
	Alloc();
	Copy(value);
}

Pu::HeightMap::HeightMap(HeightMap && value)
	: width(value.width), height(value.height),
	scale(value.scale), iscale(value.iscale),
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
		scale = other.scale;
		iscale = other.iscale;

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
		scale = other.scale;
		iscale = other.iscale;
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
	return pos.X < width * iscale && pos.Y < height * iscale && pos.X > 0.0f && pos.Y > 0.0f;
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
	/* Check anyway on debug to give proper errors. */
#ifdef _DEBUG
	if (!Contains(pos))
	{
		Log::Fatal("Cannot access height at [%f, %f] (out of [%f, %f] range)!", pos.X, pos.Y, width * iscale, height * iscale);
	}
#endif

	/* Get the patch location from the terrain position. */
	const size_t px = ipart(pos.X * scale);
	const size_t py = ipart(pos.Y * scale);

	/* Get the relative position on the patch. */
	const float x = fmodf(pos.X, iscale) * scale;
	const float y = fmodf(pos.Y, iscale) * scale;

	/* We might be on the edge, if this is the case; just return a non interpolated value. */
	if (px >= boundX || py >= boundY) return GetHeight(min(px, boundX), min(py, boundY));

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	float a, b, c = GetHeight(px, py + 1);
	if (x <= (1.0f - y))
	{
		a = GetHeight(px, py);
		b = GetHeight(px + 1, py);
	}
	else
	{
		a = GetHeight(px + 1, py);
		b = GetHeight(px + 1, py + 1);
	}

	/* Return the interpolated position. */
	return barycentric(a, b, c, x, -x - (y - 1.0f));
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
	/* Do some checks on debug mode, so easier bug catching. */
#ifdef _DEBUG
	if (!Contains(pos))
	{
		Log::Fatal("Cannot access normal at [%f, %f] (out of [%f, %f] range)!", pos.X, pos.Y, width * iscale, height * iscale);
	}

	/* We don't lazily create the normals now, that would add a branch to every height query. */
	if (!normals) Log::Fatal("Cannot access normal at [%f, %f] (normals have not been calculated)!", pos.X, pos.Y);
#endif

	/* Get the patch location from the terrain position. */
	const size_t px = ipart(pos.X * scale);
	const size_t py = ipart(pos.Y * scale);

	/* Get the relative position on the patch. */
	const float x = fmodf(pos.X, iscale) * scale;
	const float y = fmodf(pos.Y, iscale) * scale;

	/* We might be on the edge, if this is the case; just return a non interpolated value. */
	if (px >= boundX || py >= boundY) return GetNormal(min(px, boundX), min(py, boundY));

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	Vector3 a, b, c = GetNormal(px, py + 1);
	if (x <= (1.0f - y))
	{
		a = GetNormal(px, py);
		b = GetNormal(px + 1, py);
	}
	else
	{
		a = GetNormal(px + 1, py);
		b = GetNormal(px + 1, py + 1);
	}

	/* Return the interpolated position. */
	return normalize(barycentric(a, b, c, x, -x - (y - 1.0f)));
}

void Pu::HeightMap::GetHeightAndNormal(Vector2 pos, float & output, Vector3 & normal) const
{
	/* Do some checks on debug mode, so easier bug catching. */
#ifdef _DEBUG
	if (!Contains(pos))
	{
		Log::Fatal("Cannot access height and normal at [%f, %f] (out of [%f, %f] range)!", pos.X, pos.Y, width * iscale, height * iscale);
	}

	/* We don't lazily create the normals now, that would add a branch to every height query. */
	if (!normals) Log::Fatal("Cannot access normal at [%f, %f] (normals have not been calculated)!", pos.X, pos.Y);
#endif

	/* Get the patch location from the terrain position. */
	const size_t px = ipart(pos.X * scale);
	const size_t py = ipart(pos.Y * scale);

	/* Get the relative position on the patch. */
	const float x = fmodf(pos.X, iscale) * scale;
	const float y = fmodf(pos.Y, iscale) * scale;

	/* We might be on the edge, if this is the case; just return a non interpolated values. */
	if (px >= boundX || py >= boundY)
	{
		const size_t cx = min(px, boundX);
		const size_t cy = min(py, boundY);

		output = GetHeight(cx, cy);
		normal = GetNormal(cx, cy);
		return;
	}

	/* Calculate the barycentric location from the correct traingle in the heightmap. */
	float ah, bh, ch = GetHeight(px, py + 1);
	Vector3 an, bn, cn = GetNormal(px, py + 1);
	if (x <= (1.0f - y))
	{
		ah = GetHeight(px, py);
		bh = GetHeight(px + 1, py);

		an = GetNormal(px, py);
		bn = GetNormal(px + 1, py);
	}
	else
	{
		ah = GetHeight(px + 1, py);
		bh = GetHeight(px + 1, py + 1);

		an = GetNormal(px + 1, py);
		bn = GetNormal(px + 1, py + 1);
	}

	/* Return the interpolated values. */
	const float v = -x - (y - 1.0f);
	output = barycentric(ah, bh, ch, x, v);
	normal = normalize(barycentric(an, bn, cn, x, v));
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