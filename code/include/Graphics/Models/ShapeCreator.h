#pragma once
#include "Mesh.h"

namespace Pu
{
	/* Defines a helper class for generating basic shapes, all meshes are genered with the Basic3D vertex format. */
	class ShapeCreator
	{
	public:
		ShapeCreator(void) = delete;
		ShapeCreator(_In_ const ShapeCreator&) = delete;
		ShapeCreator(_In_ ShapeCreator&&) = delete;

		_Check_return_ ShapeCreator& operator =(_In_ const ShapeCreator&) = delete;
		_Check_return_ ShapeCreator& operator =(_In_ ShapeCreator&&) = delete;

		/* Gets the required buffer size (in bytes) for a plane. */
		_Check_return_ static size_t GetPlaneBufferSize(void);
		/* Populates the source buffer with a basic plane and returns a mesh that can view that plane in the destination buffer. */
		_Check_return_ static Mesh Plane(_In_ Buffer &src, _In_ Buffer &dst);

	private:
		static bool CheckSrcBuffer(Buffer &buffer, size_t requiredSize);
	};
}