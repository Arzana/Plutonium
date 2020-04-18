#include "Core/Math/HeightMap.h"
#include "Core/Math/Interpolation.h"
#include "Core/Diagnostics/Logging.h"

Pu::HeightMap::HeightMap(size_t dimensions, float patchSize)
	: HeightMap(dimensions, dimensions, patchSize)
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
	Copy(value.data);
}

Pu::HeightMap::HeightMap(HeightMap && value)
	: width(value.width), height(value.height),
	scale(value.scale), iscale(value.iscale),
	boundX(value.boundX), boundY(value.boundY),
	data(value.data)
{
	value.data = nullptr;
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

		data = reinterpret_cast<float*>(realloc(data, other.width * other.height * sizeof(float)));
		Copy(other.data);
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
		data = other.data;
		iscale = other.iscale;

		other.data = nullptr;
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
		Log::Fatal("Cannot get height at [%f, %f] in heightmap (out of [%f, %f] range)!", pos.X, pos.Y, width * iscale, height * iscale);
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

void Pu::HeightMap::Alloc(void)
{
	data = reinterpret_cast<float*>(malloc(width * height * sizeof(float)));
}

void Pu::HeightMap::Copy(const float * other)
{
	memcpy(data, other, width * height * sizeof(float));
}

void Pu::HeightMap::Free(void)
{
	if (data) free(data);
}