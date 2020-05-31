#pragma once
#include "Mesh.h"

namespace Pu
{
	/* Defines a collection of meshes combined into one GPU data buffer. */
	class MeshCollection
	{
	public:
		/* Defines the material index used for default materials. */
		static constexpr uint32 DefaultMaterialIdx = ~0u;
		/* Defines a mesh with a material index. */
		using Shape = std::pair<uint32, Mesh>;

		/* Initializes an empty instance of a mesh collection. */
		MeshCollection(void);
		/* Initializes a new instance of a mesh collection from the specified PuM source. */
		MeshCollection(_In_ LogicalDevice &device, _In_ const PuMData &data);
		MeshCollection(_In_ const MeshCollection&) = delete;
		/* Move constructor. */
		MeshCollection(_In_ MeshCollection &&value);
		/* Releases the GPU resource. */
		~MeshCollection(void)
		{
			Destroy();
		}

		_Check_return_ MeshCollection& operator =(_In_ const MeshCollection&) = delete;
		/* Move assignment. */
		_Check_return_ MeshCollection& operator =(_In_ MeshCollection &&other);

		/* Initializes the mesh collection from a PuM source if it hasn't been initialized yet. */
		void Initialize(_In_ LogicalDevice &device, _In_ const PuMData &data);
		/* Initializes the mesh collection with a single mesh from the specified source buffer. */
		void Initialize(_In_ LogicalDevice &device, _In_ StagingBuffer &src, _In_ uint32 vrtxSize, _In_ Mesh &&mesh);
		/* Initializes the mesh collection with a single mesh from the specified source buffer. */
		void Initialize(_In_ LogicalDevice &device, _In_ _In_ uint32 idxSize, _In_ uint32 vrtxSize, _In_ Mesh &&mesh);
		/* Binds the specified mesh directly to the command buffer, ignoring possible view optimizations. */
		void Bind(_In_ CommandBuffer &cmdBuffer, _In_ uint32 binding, _In_ uint32 mesh) const;

		/* Gets an AABB that is a combination of all mesh bounding boxes. */
		_Check_return_ inline AABB GetBoundingBox(void) const
		{
			return boundingBox;
		}

		/* Gets the bounding box of the specified child mesh. */
		_Check_return_ inline AABB GetBoundingBox(_In_ uint32 mesh) const
		{
			return meshes.at(mesh).second.GetBoundingBox();
		}

		/* Gets the offset into the GPU data at the specific view start location. */
		_Check_return_ inline DeviceSize GetViewOffset(_In_ uint32 viewIdx) const
		{
			return views.at(viewIdx).Offset;
		}

		/* Gets the mesh and matrial index at the specified index. */
		_Check_return_ inline const Shape& GetShape(_In_ uint32 index) const
		{
			return meshes.at(index);
		}

		/* Gets the GPU memory buffer. */
		_Check_return_ inline Buffer& GetBuffer(void)
		{
			return *memory;
		}

		/* Gets the GPU memory buffer. */
		_Check_return_ inline const Buffer& GetBuffer(void) const
		{
			return *memory;
		}

		/* Allows for the use of range based for loops. */
		_Check_return_ inline vector<Shape>::const_iterator begin(void) const
		{
			return meshes.begin();
		}

		/* Allows for the use of range based for loops. */
		_Check_return_ inline vector<Shape>::const_iterator end(void) const
		{
			return meshes.end();
		}

		/* Gets the amount of meshes stored in this collection. */
		_Check_return_ inline size_t Count(void) const
		{
			return meshes.size();
		}

	private:
		Buffer *memory;
		vector<PumView> views;
		vector<Shape> meshes;
		AABB boundingBox;

		void Alloc(LogicalDevice &device, size_t size);
		void SetBoundingBox(void);
		void Destroy(void);
	};
}