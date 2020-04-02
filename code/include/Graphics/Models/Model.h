#pragma once
#include "Category.h"
#include "Content/Asset.h"
#include "Mesh.h"
#include "Material.h"

namespace Pu
{
	class Texture2D;
	class DeferredRenderer;
	class LightProbeRenderer;

	/* Defines the base class for a model with multiple meshes and materials. */
	class Model
		: public Asset
	{
	public:
		/* Defines the material index used for default materials. */
		static constexpr uint32 DefaultMaterialIdx = ~0u;
		/* Defines the combination of a mesh and a material index. */
		using Shape = std::pair<uint32, Mesh>;

		/* Defines the category of this model. */
		ModelCategory Category;

		/* Initializes a new instance of a model. */
		Model(void);
		Model(_In_ const Model&) = delete;
		/* Move constructor. */
		Model(_In_ Model &&value);
		/* Releases the resources allocated by the model. */
		virtual ~Model(void)
		{
			Destroy();
		}

		_Check_return_ Model& operator =(_In_ const Model&) = delete;
		/* Move assignment. */
		_Check_return_ Model& operator =(_In_ Model &&other);

		/* Gets the meshes (with vertex format of Basic3D) in this model. */
		_Check_return_ inline const vector<Shape>& GetBasicMeshes(void) const
		{
			return basicMeshes;
		}

		/* Gets the meshes (with vertex format of Advanced3D) in this model. */
		_Check_return_ inline const vector<Shape>& GetAdvancedMeshes(void) const
		{
			return advancedMeshes;
		}

		/* Gets the material at the specified index. */
		_Check_return_ inline const Material& GetMaterial(_In_ uint32 idx) const
		{
			return materials.at(idx);
		}

		/* Gets the material used to render the light probes at the specified index. */
		_Check_return_ inline const DescriptorSet& GetProbeMaterial(_In_ uint32 idx) const
		{
			return probeMaterials.at(idx);
		}

		/* Gets the bounding box of this model. */
		_Check_return_ inline AABB GetBoundingBox(void) const
		{
			return boundingBox;
		}

		/* Gets the offset into the GPU data at which the specified view starts. */
		_Check_return_ inline DeviceSize GetViewOffset(_In_ uint32 idx) const
		{
			return views.at(idx).Offset;
		}

		/* Gets the GPU data buffer. */
		_Check_return_ inline const Buffer& GetBuffer(void) const
		{
			return *gpuData;
		}

	protected:
		/* Increases the reference counter by one and returns itself. */
		_Check_return_ virtual Asset& Duplicate(_In_ AssetCache&);

	private:
		friend class AssetLoader;
		friend class AssetFetcher;

		DescriptorPool *poolMaterials, *poolProbes;
		Buffer *gpuData;
		AABB boundingBox;

		vector<PumNode> nodes;
		vector<Shape> basicMeshes;
		vector<Shape> advancedMeshes;
		vector<Material> materials;
		vector<DescriptorSet> probeMaterials;
		vector<Texture2D*> textures;
		vector<PumView> views;

		void AllocBuffer(LogicalDevice &device, const StagingBuffer &buffer);
		void AllocPools(const DeferredRenderer &deferred, const LightProbeRenderer &probes, size_t count);
		void Initialize(LogicalDevice &device, const PuMData &data);
		void Finalize(CommandBuffer &cmdBuffer, const DeferredRenderer &deferred, const LightProbeRenderer &probes, const PuMData &data);
		Material& AddMaterial(size_t diffuse, size_t specular, size_t normal, const DeferredRenderer &deferred, const LightProbeRenderer &probes);
		void CalculateBoundingBox(void);
		void Destroy(void);
	};
}