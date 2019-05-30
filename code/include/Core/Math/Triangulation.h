#pragma once
#include <sal.h>

namespace Pu
{
	/* Triangulates a quad into two triangles, input vertices must contain 4 vertices and outpuyt will always contain tho objects. */
	void triangulateQuad(_In_ void *vertices, _In_ size_t stride, _In_ size_t offset);
	/* Triangulates a convex polygon into n-2 triangles (where n is the number of edges), returns the number of triangles. */
	_Check_return_ size_t triangulateConvex(_In_ void *vertices, _In_ size_t stride, _In_ size_t count);
}