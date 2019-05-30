#include "Core/Math/Triangulation.h"
#include "Core/Math/Vector3.h"
#include "Core/Diagnostics/Logging.h"

using namespace Pu;

/* We have to cast to char pointers because C++ doesn't allow arithmatic on void pointers. */
#define ascp(x)			reinterpret_cast<byte*>(x)
#define asccp(x)		reinterpret_cast<const byte*>(x)

/* Gets the vertex at the specified offset. */
static inline Vector3 vrtxat(const void *vertexes, size_t stride, size_t offset, size_t idx)
{
	return *reinterpret_cast<const Vector3*>((asccp(vertexes) + (idx * stride)) + offset);
}

/* Copies the vertex at a specified location to the destination buffer, where i is the source index and j is the destination index. */
static inline void setvrtx(void *dest, const void *src, size_t stride, size_t i, size_t j)
{
	memcpy(ascp(dest) + stride * i, asccp(src) + stride * j, stride);
}

void Pu::triangulateQuad(void * vertices, size_t stride, size_t offset)
{
	/* On debug, check for invalid arguments. */
#ifdef _DEBUG
	if (!vertices) Log::Fatal("Vertex buffer cannot bu null!");
	if (stride < sizeof(Vector3)) Log::Fatal("Stride must be at least %zu!", sizeof(Vector3));
#endif

	/* Make a copy of the input buffer. */
	const size_t size = 6 * stride;
	void *temp = malloc(size);
	memcpy(temp, vertices, size);

	/* Get the quad vertices. */
	Vector3 v1 = vrtxat(vertices, stride, offset, 0);
	Vector3 v2 = vrtxat(vertices, stride, offset, 1);
	Vector3 v3 = vrtxat(vertices, stride, offset, 2);
	Vector3 v4 = vrtxat(vertices, stride, offset, 3);

	/* Calculate shortes distance, we can use square distance because we are only comparing distances. */
	float d1 = sqrdist(v1, v3);
	float d2 = sqrdist(v2, v4);

	if (d1 > d2)
	{
		/*
		Split along b-d axis.
		The first two vertices stay the same so we don't have to copy them.
		*/
		setvrtx(vertices, temp, stride, 2, 3);

		setvrtx(vertices, temp, stride, 3, 1);
		setvrtx(vertices, temp, stride, 4, 2);
		setvrtx(vertices, temp, stride, 5, 3);
	}
	else
	{
		/*
		Split along a-c axis.
		The first triangle stays the same so we don't have to copy it.
		*/

		setvrtx(vertices, temp, stride, 3, 0);
		setvrtx(vertices, temp, stride, 4, 2);
		setvrtx(vertices, temp, stride, 5, 3);
	}

	/* Free temporary buffer. */
	free(temp);
}

size_t Pu::triangulateConvex(void * vertices, size_t stride, size_t count)
{
	/* On debug, check for invalid arguments. */
#ifdef _DEBUG
	if (!vertices) Log::Fatal("Vertex buffer cannot bu null!");
	if (stride < sizeof(Vector3)) Log::Fatal("Stride must be at least %zu!", sizeof(Vector3));
	if (count < 3) Log::Fatal("Polygon has to have at least 3 vertices (%zu specified)!", count);
#endif

	/* Make a copy of the input buffer for later swapping. */
	const size_t size = (count - 2) * 3;
	void *temp = malloc(size * stride);
	memcpy(temp, vertices, count * stride);

	/* Fan through all vertices to traingulate convex polygon. */
	for (size_t i = 1, j = 0; i < count - 1; i++)
	{
		setvrtx(vertices, temp, stride, j++, 0);
		setvrtx(vertices, temp, stride, j++, i);
		setvrtx(vertices, temp, stride, j++, i + 1);
	}

	/* Free temporary buffer. */
	free(temp);
	return size;
}