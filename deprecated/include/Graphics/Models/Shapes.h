#pragma once
#include "Graphics\Mesh.h"

namespace Plutonium
{
	/* Defines a helper structure for creating position data meshes of pre-defined simple shapes. */
	class ShapeCreator
	{
	public:
		/* Populates the specified buffer with a plane. */
		_Check_return_ static void MakePlane(_In_ Mesh *mesh, _In_ Vector2 scale);
		/* Populates the specified buffer with a box. */
		_Check_return_ static void MakeBox(_In_ Mesh *mesh, _In_ Vector3 scale);
		/* Populates the specified buffer with a sphere. */
		_Check_return_ static void MakeSphere(_In_ Mesh *mesh, _In_ size_t meridians, _In_ size_t parallels, _In_ float radius);
		/* Populates the specified buffer with a pyramid. */
		_Check_return_ static void MakePyramid(_In_ Mesh *mesh, _In_ Vector2 base, _In_ float height);

	private:
		static void SetTangents(Mesh *mesh);
	};
}