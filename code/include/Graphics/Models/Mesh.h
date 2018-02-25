#pragma once
#include "MeshVertexFormat.h"
#include "Core\Math\Constants.h"
#include <vector>

namespace Plutonium
{
	struct LoaderResult;

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
		_Check_return_ uint32 GetVertexBuffer(void) const;
		/* Gets a vertex at a specified position. */
		_Check_return_ VertexFormat& GetVertexAt(_In_ size_t idx) const;
		/* Gets the amount of vertices stored by the mesh. */
		_Check_return_ inline size_t GetVertexCount(void) const
		{
			return vrtxCnt;
		}

	private:
		friend struct Model;

		VertexFormat *vertices;
		size_t vrtxCnt;
		uint32 ptr;

		static Mesh* FromFile(const LoaderResult *buffer, size_t idx);
	};
}