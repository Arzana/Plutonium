#pragma once
#include "Mesh.h"
#include "Graphics/VertexLayouts/Basic3D.h"
#include "Graphics/VertexLayouts/Patched3D.h"

namespace Pu
{
	/* Defines a helper class for generating basic shapes, all meshes are genered with the Basic3D vertex format. */
	class ShapeCreator
	{
	public:
		/* Defines the size (in bytes) of the vertices generated by the plane. */
		static constexpr uint32 PlaneVertexSize = 4 * sizeof(Basic3D);
		/* Defines the size (in bytes) of the vertices generated by the box. */
		static constexpr uint32 BoxVertexSize = PlaneVertexSize * 6;
		/* 
		Defines the required buffer size (in bytes) for a plane. 
		The plane has 4 vertices (because it's a quad) and 6 indices (for two triangles).
		*/
		static constexpr size_t PlaneBufferSize = PlaneVertexSize + 6 * sizeof(uint16);
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

		/* Gets the size (in bytes) of the vertices generated by the path list plane. */
		_Check_return_ static uint32 GetPatchPlaneVertexSize(_In_ uint16 divisions);
		/* Gets the size (in bytes) of the vertices generated by the sphere. */
		_Check_return_ static uint32 GetSphereVertexSize(_In_ uint16 divisions);
		/* Gets the size (in bytes) of the vertices generated by the hemisphere. */
		_Check_return_ static uint32 GetDomeVertexSize(_In_ uint16 divisions);
		/* Gets the size (in bytes) of the vertices generated by the torus. */
		_Check_return_ static uint32 GetTorusVertexSize(_In_ uint16 divisions);
		/* Gets the size (in bytes) of the vertices generated by the cylinder. */
		_Check_return_ static uint32 GetCylinderVertexSize(_In_ uint16 divisions);

		/* Gets the required buffer size (in bytes) for a patch list plane. */
		_Check_return_ static size_t GetPatchPlaneBufferSize(_In_ uint16 divisions);
		/* Gets the required buffer size (in bytes) for a sphere. */
		_Check_return_ static size_t GetSphereBufferSize(_In_ uint16 divisions);
		/* Gets the required buffer size (in bytes) for a hemisphere. */
		_Check_return_ static size_t GetDomeBufferSize(_In_ uint16 divisions);
		/* Gets the required buffer size (in bytes) for a torus. */
		_Check_return_ static size_t GetTorusBufferSize(_In_ uint16 divisions);
		/* Gets the required buffer size (in bytes) for a cylinder. */
		_Check_return_ static size_t GetCylinderBufferSize(_In_ uint16 divisions);

		/* Populates the source buffer with a basic plane and returns a mesh that can view that plane. */
		_Check_return_ static Mesh Plane(_In_ Buffer &src);
		/* Populates the source buffer with a patch list plane and returns a mesh that can view that plane. */
		_Check_return_ static Mesh PatchPlane(_In_ Buffer &src, _In_ uint16 divisions);
		/* Populates the source buffer with a basic box and returns a mesh that can view that box. */
		_Check_return_ static Mesh Box(_In_ Buffer &src);
		/* Populates the source buffer with a spherified cube and returns a mesh that can view that sphere. */
		_Check_return_ static Mesh Sphere(_In_ Buffer &src, _In_ uint16 divisions);
		/* Populates the source buffer with a (UV) hemisphere and returns a mesh that can view that hemisphere. */
		_Check_return_ static Mesh Dome(_In_ Buffer &src, _In_ uint16 divisions);
		/* Populates the source buffer with a torus and returns a mesh that can view that torus. */
		_Check_return_ static Mesh Torus(_In_ Buffer &src, _In_ uint16 divisions, _In_ float ratio);
		/* Populates the source buffer with a cylinder and returns a mesh that can view that cylinder. */
		_Check_return_ static Mesh Cylinder(_In_ Buffer &src, _In_ uint16 divisions);

	private:
#ifdef _DEBUG
		static bool CheckSrcBuffer(Buffer &buffer, size_t requiredSize);
#endif
	};
}