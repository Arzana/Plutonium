#pragma once
#include "Graphics/Resources/BufferAccessor.h"

namespace Pu
{
	/* Defines a helper object for creating common meshes. */
	class MeshCreator
	{
	public:
		MeshCreator(void) = delete;
		MeshCreator(_In_ const MeshCreator&) = delete;
		MeshCreator(_In_ MeshCreator&&) = delete;

		_Check_return_ MeshCreator& operator =(_In_ const MeshCreator&) = delete;
		_Check_return_ MeshCreator& operator =(_In_ MeshCreator&&) = delete;

		/* Creates a new 2D rectangle with the specified accessors. */
		static void CreateRectangle(_In_ BufferAccessor *positions, _In_ BufferAccessor *texCoords, _In_ const Matrix &transform);
		/* Creates a new 3D plane with the specified accessors. */
		static void CreatePlane(_In_ BufferAccessor *positions, _In_ BufferAccessor *normals, _In_ BufferAccessor *texCoords, _In_ BufferAccessor *tangents, _In_ const Matrix &transform);
		/* Creates a new 3D box with the specified accessors. */
		static void CreateBox(_In_ BufferAccessor *positions, _In_ BufferAccessor *normals, _In_ BufferAccessor *texCoords, _In_ BufferAccessor *tangents, _In_ const Matrix &transform);
		/* Creates a new 3D sphere with the specified accessors. */
		static void CreateSphere(_In_ BufferAccessor *positions, _In_ BufferAccessor *normals, _In_ BufferAccessor *texCoords, _In_ BufferAccessor *tangents, _In_ size_t meridians, _In_ size_t parallels, _In_ const Matrix &transform);
		/* Creates a new 3D pyramid with the specified accessors.*/
		static void CreatePyramid(_In_ BufferAccessor *positions, _In_ BufferAccessor *normals, _In_ BufferAccessor *texCoords, _In_ BufferAccessor *tangents, _In_ const Matrix &transform);

	private:
		static FieldType pos3DType;
		static FieldType texCoordType;

		static void InvalidAccessor(const BufferAccessor &accessor, FieldType requiredType);
	};
}