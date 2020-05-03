#pragma once
#include "Graphics/Vulkan/CommandBuffer.h"
#include "Content/PumLoader.h"

namespace Pu
{
	/* Defines a container for a vertex and index buffer view. */
	class Mesh
	{
	public:
		/* Defines the view index used if no view is used. */
		static constexpr uint32 DefaultViewIdx = ~0u;

		/* Default initializes the mesh. */
		Mesh(void);
		/* Initializes a new instance of a mesh with a specific vertex and index buffer. */
		Mesh(_In_ uint32 indexCount, _In_ uint32 vertexView, _In_ uint32 indexView, _In_ uint32 firstIndex, _In_ size_t vertexStride, _In_ IndexType indexType);
		/* Initializes a new instance of a mesh with a specific stride and type (views will be set to zero). */
		Mesh(_In_ uint32 indexCount, _In_ size_t vertexStride, _In_ IndexType indexType);
		/* Initializes a new instance of a mesh from a PuM mesh. */
		Mesh(_In_ const PumMesh &mesh);
		/* Copy constructor. */
		Mesh(_In_ const Mesh &value) = default;
		/* Move constructor. */
		Mesh(_In_ Mesh &&value) = default;

		/* Copy assignment. */
		_Check_return_ Mesh& operator =(_In_ const Mesh &other) = default;
		/* Move assignment. */
		_Check_return_ Mesh& operator =(_In_ Mesh &&other) = default;

		/* Draws the mesh a specified amount of times. */
		void Draw(_In_ CommandBuffer &cmdBuffer, _In_ uint32 instanceCount) const;

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
			return stride;
		}

		/* Gets the amount of indices or vertices that need to be drawn for this mesh. */
		_Check_return_ inline uint32 GetCount(void) const
		{
			return count;
		}

		/* Gets the type of indices used by this mesh. */
		_Check_return_ inline IndexType GetIndexType(void) const
		{
			return type;
		}

		/* Gets the index of the vertex view used by the mesh. */
		_Check_return_ inline uint32 GetVertexView(void) const
		{
			return vertexView;
		}

		/* Gets the index of the index view used by the mesh. */
		_Check_return_ inline uint32 GetIndexView(void) const
		{
			return indexView;
		}

		/* Gets the offset (in bytes) from the index or vertex view where the first vertex is located. */
		_Check_return_ inline DeviceSize GetBindOffset(void) const
		{
			return first * stride;
		}

	private:
		IndexType type;
		AABB boundingBox;
		int32 offset;
		uint32 stride, count, first;
		uint32 vertexView, indexView;
	};
}