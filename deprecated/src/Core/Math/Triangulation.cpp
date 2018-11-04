#include "Core\Math\Triangulation.h"
#include "Core\SafeMemory.h"

using namespace Plutonium;

/* We have to cast to char pointers because C++ doesn't allow arithmatic on void pointers. */
#define ascp(x)			reinterpret_cast<char*>(x)
#define asccp(x)		reinterpret_cast<const char*>(x)

/* Gets the vertex at the specified offset. */
_Check_return_ inline Vector3 vrtxat(_In_ const void *vertexes, _In_ size_t stride, _In_ size_t offset, _In_ size_t idx)
{
	return *reinterpret_cast<const Plutonium::Vector3*>((asccp(vertexes) + (idx * stride)) + offset);
}

/* Copies the vertex at a specified location to the destination buffer, where i is the source index and j is the destination index. */
inline void setvrtx(_In_ void *dest, _In_ const void *src, _In_ size_t stride, _In_ size_t i, _In_ size_t j)
{
	memcpy(ascp(dest) + stride * i, asccp(src) + stride * j, stride);
}

void Plutonium::Triangulation::Quad(void * vertexes, size_t stride, size_t offset)
{
	/* Check for invalid arguments. */
	ASSERT_IF(!vertexes, "Vertex buffer cannot be null!");
	ASSERT_IF(stride < sizeof(Vector3), "Stride must be at least %zu!", sizeof(Vector3));

	/* Make a copy of the input buffer for later swapping. */
	size_t size = 6 * stride;
	void *temp = malloca_s(char, size);
	memcpy(temp, vertexes, size);

	/* Get the quad vertices. */
	Vector3 v1 = vrtxat(vertexes, stride, offset, 0);
	Vector3 v2 = vrtxat(vertexes, stride, offset, 1);
	Vector3 v3 = vrtxat(vertexes, stride, offset, 2);
	Vector3 v4 = vrtxat(vertexes, stride, offset, 3);

	/* Calculate shortes distance, we can use square distance because we are only comparing distances. */
	float d1 = sqrdist(v1, v3);
	float d2 = sqrdist(v2, v4);

	if (d1 > d2)
	{
		/* 
		Split along b-d axis.
		The first two vertices stay the same so we don't have to copy them.
		*/
		setvrtx(vertexes, temp, stride, 2, 3);

		setvrtx(vertexes, temp, stride, 3, 1);
		setvrtx(vertexes, temp, stride, 4, 2);
		setvrtx(vertexes, temp, stride, 5, 3);
	}
	else
	{
		/* 
		Split along a-c axis. 
		The first triangle stays the same so we don't have to copy it.
		*/
		
		setvrtx(vertexes, temp, stride, 3, 0);
		setvrtx(vertexes, temp, stride, 4, 2);
		setvrtx(vertexes, temp, stride, 5, 3);
	}

	/* Free temporary buffer. */
	freea_s(temp);
}

size_t Plutonium::Triangulation::Convex(void * vertexes, size_t stride, size_t count)
{
	/* Check for invalid arguments. */
	ASSERT_IF(!vertexes, "Vertex buffer cannot be null!");
	ASSERT_IF(stride < sizeof(Vector3), "Stride must be at least %zu!", sizeof(Vector3));
	ASSERT_IF(count < 3, "Cannot triangulate polygon with less than 3 vertices!");

	/* Make a copy of the input buffer for later swapping. */
	const size_t size = (count - 2) * 3;
	void *temp = malloca_s(char, size * stride);
	memcpy(temp, vertexes, count * stride);

	/* Fan through all vertices to traingulate convex polygon. */
	for (size_t i = 1, j = 0; i < count - 1; i++)
	{
		setvrtx(vertexes, temp, stride, j++, 0);
		setvrtx(vertexes, temp, stride, j++, i);
		setvrtx(vertexes, temp, stride, j++, i + 1);
	}

	/* Free temporary buffer. */
	freea_s(temp);
	return size;
}