#pragma once
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Content/PumLoader.h"

namespace Pu
{
	/* Defines a container for a vertex and index buffer view. */
	class Mesh
	{
	public:
		/* Default initializes the mesh. */
		Mesh(void);
		/* Initializes a new instance of a mesh with a specific vertex and index buffer. */
		Mesh(_In_ const Buffer &buffer, _In_ size_t vertexOffset, _In_ size_t vertexSize, _In_ size_t indexOffset, _In_ size_t indexSize, _In_ size_t vertexStride, _In_ size_t indexStride, _In_ IndexType indexType);
		/* Initializes a new instance of a mesh with a specific vertex and index buffer (vertex offset is zero and index offset is vertex size). */
		Mesh(_In_ const Buffer &buffer, _In_ size_t vertexSize, _In_ size_t indexSize, _In_ size_t vertexStride, _In_ size_t indexStride, _In_ IndexType indexType);
		/* Initializes a new instance of a mesh from a PuM mesh. */
		Mesh(_In_ const Buffer &buffer, _In_ const PumMesh &mesh);
		/* Copy constructor. */
		Mesh(_In_ const Mesh &value);
		/* Move constructor. */
		Mesh(_In_ Mesh &&value);
		/* Releases the resources allocated by the mesh. */
		~Mesh(void)
		{
			Destroy();
		}

		/* Copy assignment. */
		_Check_return_ Mesh& operator =(_In_ const Mesh &other);
		/* Move assignment. */
		_Check_return_ Mesh& operator =(_In_ Mesh &&other);

		/* Binds the vertex buffer to the specific binding and binds the index buffer. */
		void Bind(_In_ CommandBuffer &cmdBuffer, _In_ uint32 binding) const;
		/* Draws the mesh a specified amount of times. */
		void DrawInstanced(_In_ CommandBuffer &cmdBuffer, _In_ uint32 instanceCount) const;
		/* Draws the mesh once. */
		inline void Draw(_In_ CommandBuffer &cmdBuffer) const
		{
			DrawInstanced(cmdBuffer, 1);
		}

		/* Sets the bounding box of this mesh (automatically set when using PuM). */
		inline void SetBoundingBox(_In_ AABB bb)
		{
			boundingBox = bb;
		}

		/* Gets the bounding box of this mesh. */
		_Check_return_ inline AABB GetBoundingBox(void) const
		{
			return boundingBox;
		}

		/* Gets the stride (in bytes) of the vertices in this mesh. */
		_Check_return_ inline uint32 GetStride(void) const
		{
			return static_cast<uint32>(vertex->GetStride());
		}

	private:
		const BufferView *vertex;
		const BufferView *index;
		IndexType type;
		bool useIndexBuffer;
		AABB boundingBox;

		void Check(void) const;
		void Destroy(void);
	};
}