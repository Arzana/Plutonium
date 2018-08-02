#pragma once
#include <vector>
#include "Vector3.h"

namespace Plutonium
{
	/* Defines functions for basic triangulation. */
	struct Triangulation
	{
	public:
		/* Triangulates a quad into two triangles, input vertices must contains 4 vertices and output will always contain two objects. */
		static void Quad(_In_ void *vertexes, _In_ size_t stride, _In_ size_t offset);
		/* Triangulates a convex polygon into n-2 triangles (where n is the number of edges), returns the number of triangles. */
		_Check_return_ static size_t Convex(_In_ void *vertexes, _In_ size_t stride, _In_ size_t offset, _In_ size_t count);
	};
}