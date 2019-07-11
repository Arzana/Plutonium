#pragma once
#include "Mesh.h"
#include "Graphics/VertexLayouts/Basic3D.h"

namespace Pu
{
	/* Defines a helper class for generating basic shapes, all meshes are genered with the Basic3D vertex format. */
	class ShapeCreator
	{
	public:
		/* 
		Defines the required buffer size (in bytes) for a plane. 
		The plane has 4 vertices (because it's a quad) and 6 indices (for two triangles).
		*/
		static constexpr size_t PlaneBufferSize = 4 * sizeof(Basic3D) + 6 * sizeof(uint16);
		/*
		Defines the required buffer size (in bytes) for a box.
		The box is created using 6 planes instead of packed.
		This is because the normals need to be correct for each face of the box.
		*/
		static constexpr size_t BoxBufferSize = PlaneBufferSize * 6;

		ShapeCreator(void) = delete;
		ShapeCreator(_In_ const ShapeCreator&) = delete;
		ShapeCreator(_In_ ShapeCreator&&) = delete;

		_Check_return_ ShapeCreator& operator =(_In_ const ShapeCreator&) = delete;
		_Check_return_ ShapeCreator& operator =(_In_ ShapeCreator&&) = delete;

		/* Gets the required buffer size (in bytes) for a sphere. */
		_Check_return_ static size_t GetSphereBufferSize(_In_ size_t divisions);
		/* Gets the required buffer size (in bytes) for a hemisphere. */
		_Check_return_ static size_t GetDomeBufferSize(_In_ size_t divisions);

		/* Populates the source buffer with a basic plane and returns a mesh that can view that plane in the destination buffer. */
		_Check_return_ static Mesh Plane(_In_ Buffer &src, _In_ const Buffer &dst);
		/* Populates the source buffer with a basic box and returns a mesh that can view that box in the destination buffer. */
		_Check_return_ static Mesh Box(_In_ Buffer &src, _In_ const Buffer &dst);
		/* Populates the source buffer with a spherified cube and returns a mesh that can view that sphere in the destination buffer. */
		_Check_return_ static Mesh Sphere(_In_ Buffer &src, _In_ const Buffer &dst, _In_ uint16 divisions);
		/* Populates the source buffer with a (UV) hemisphere and returns a mesh that can view that hemisphere in the destination buffer. */
		_Check_return_ static Mesh Dome(_In_ Buffer &src, _In_ const Buffer &dst, _In_ uint16 divisions);

	private:
		static bool CheckSrcBuffer(Buffer &buffer, size_t requiredSize);
	};
}