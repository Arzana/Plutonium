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

		/* Adds a specific mesh to the collection. */
		void AddMesh(_In_ const Mesh &mesh);
		/* Adds a specific view to the collection. */
		_Check_return_ uint32 AddView(_In_ size_t offset, _In_ size_t size);

		/* Adds a specific mesh to the collection with specified index and vertex views. */
		void AddMesh(_In_ const Mesh &mesh, _In_ uint32 vertexStart, _In_ uint32 vertexSize, _In_ uint32 indexStart, _In_ uint32 indexSize);
		/* Adds a specific mesh to the collection from a specific staging buffer. */
		void AddMesh(_In_ const Mesh &mesh, _In_ StagingBuffer &src, _In_ uint32 vertexStart, _In_ uint32 vertexSize);
		/* Populates the mesh collection with the specified PuM source. */
		void AddMeshes(_In_ const PuMData &data, _In_ uint32 vertexStart);
		/* Finalizes the collection, locking it and making it ready for use. */
		void Finalize(_In_ LogicalDevice &device, _In_ DeviceSize size);
		/* Binds the specified mesh directly to the command buffer, ignoring possible view optimizations. */
		void Bind(_In_ CommandBuffer &cmdBuffer, _In_ uint32 binding, _In_ uint32 mesh) const;

		/* Initializes the mesh collection from a single PuM source. */
		inline void Initialize(_In_ LogicalDevice &device, _In_ const PuMData &data)
		{
			AddMeshes(data, 0u);
			Finalize(device, data.Buffer->GetSize());
		}

		/* Initializes the mesh collection from a single source mesh. */
		inline void Initialize(_In_ LogicalDevice &device, _In_ StagingBuffer &src, _In_ uint32 vertexSize, _In_ const Mesh &mesh)
		{
			AddMesh(mesh, src, 0u, vertexSize);
			Finalize(device, src.GetSize());
		}

		/* Gets the mesh at the specified index. */
		_Check_return_ const Mesh& GetMesh(_In_ size_t idx) const
		{
			return meshes[idx].second;
		}

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

		/* Gets whether this mesh collection is fully loaded on the GPU. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return memory && memory->IsLoaded();
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