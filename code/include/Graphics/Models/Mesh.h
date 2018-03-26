#pragma once
#include "MeshVertexFormat.h"
#include "Graphics\Native\Buffer.h"
#include <vector>

namespace Plutonium
{
	struct ObjLoaderResult;
	struct PobjLoaderResult;
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
		void Finalize(void);

		/* Gets the ID for the vertex array buffer. */
		_Check_return_ inline Buffer* GetVertexBuffer(void) const
		{
			return buffer;
		}
		/* Makes sure the vertex buffer is of a specified size. */
		void SetBufferSize(_In_ size_t size);
		/* Gets a vertex at a specified position. */
		_Check_return_ VertexFormat& GetVertexAt(_In_ size_t idx) const;
		/* Gets the amount of vertices stored by the mesh. */
		_Check_return_ inline size_t GetVertexCount(void) const
		{
			return vrtxCnt;
		}

	private:
		friend struct StaticModel;
		friend struct EuclidRoom;
		friend struct DynamicModel;

		VertexFormat *vertices;
		size_t vrtxCnt;
		Buffer *buffer;

		static Mesh* FromFile(const ObjLoaderResult *buffer, size_t idx);
		static Mesh* RFromFile(const PobjLoaderResult *buffer, size_t ridx, size_t sidx);
		static Mesh* PFromFile(const PobjLoaderResult *buffer, size_t ridx, size_t pidx);
		static Mesh* FromFile(const Md2LoaderResult *buffer, size_t idx);
	};
}