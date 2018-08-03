#pragma once
#include "MeshVertexFormat.h"
#include "Graphics\Native\Buffer.h"
#include "Core\Math\Box.h"
#include <vector>

namespace Plutonium
{
	struct ObjLoaderResult;
	struct Md2LoaderResult;

	/* Defines a object for storing the vertex information for a single textured object. */
	struct Mesh
	{
	public:
		/* The name of the mesh defined by the file. */
		const char * Name;

		/* Initializes a new instance of an empty mesh. */
		Mesh(_In_ const char *name);
		Mesh(_In_ const Mesh &value) = delete;
		Mesh(_In_ Mesh &&value) = delete;
		/* Releases the resources allocated by the mesh. */
		~Mesh(void) noexcept;

		_Check_return_ Mesh& operator =(_In_ const Mesh &other) = delete;
		_Check_return_ Mesh& operator =(_In_ Mesh &&other) = delete;

		/* Pushes the vertices to the GPU and releases the resources on the CPU. */
		void Finalize(_In_ WindowHandler wnd);

		/* Gets the bounding box associated with this mesh. */
		_Check_return_ inline Box GetBoundingBox(void) const
		{
			return bb;
		}

		/* Gets the ID for the vertex array buffer. */
		_Check_return_ inline Buffer* GetVertexBuffer(void) const
		{
			return buffer;
		}
		/* Gets the amount of vertices stored by the mesh. */
		_Check_return_ inline size_t GetVertexCount(void) const
		{
			return vrtxCnt;
		}
		/* Makes sure the vertex buffer is of a specified size. */
		void SetBufferSize(_In_ size_t size);
		/* Gets a vertex at a specified position. */
		_Check_return_ VertexFormat* GetVertexAt(_In_ size_t idx) const;
		/* Merges a second mesh into this mesh. */
		void Append(_In_ Mesh *other);
		/* Initializes the tangent component of the three specified vertices (triangle). */
		static void SetTangent(_In_ VertexFormat &vrtx1, _In_ VertexFormat &vrtx2, _In_ VertexFormat &vrtx3);

	private:
		friend struct StaticModel;
		friend struct DynamicModel;

		VertexFormat *vertices;
		size_t vrtxCnt;
		Buffer *buffer;
		Box bb;

		void SetBufferSizeInternal(_In_ size_t size);

		static void SetNormal(_In_ VertexFormat &vrtx1, _In_ VertexFormat &vrtx2, _In_ VertexFormat &vrtx3);

		static Mesh* FromFile(const ObjLoaderResult *buffer, size_t idx, bool recalcNormals);
		static Mesh* FromFile(const Md2LoaderResult *buffer, size_t idx, bool recalcNormals);
	};
}