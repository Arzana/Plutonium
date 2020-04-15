#pragma once
#include "Category.h"
#include "Material.h"
#include "Content/Asset.h"
#include "MeshCollection.h"

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

		/* Gets the underlying mesh data of this model. */
		_Check_return_ inline const MeshCollection& GetMeshes(void) const
		{
			return meshes;
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

	protected:
		/* Increases the reference counter by one and returns itself. */
		_Check_return_ virtual Asset& Duplicate(_In_ AssetCache&);

	private:
		friend class AssetLoader;
		friend class AssetFetcher;

		DescriptorPool *poolMaterials, *poolProbes;
		vector<Material> materials;
		vector<DescriptorSet> probeMaterials;
		vector<Texture2D*> textures;

		MeshCollection meshes;
		vector<PumNode> nodes;

		void AllocPools(const DeferredRenderer &deferred, const LightProbeRenderer &probes, size_t count);
		void Finalize(CommandBuffer &cmdBuffer, const DeferredRenderer &deferred, const LightProbeRenderer &probes, const PuMData &data);
		Material& AddMaterial(size_t diffuse, size_t specular, size_t normal, const DeferredRenderer &deferred, const LightProbeRenderer &probes);
		void Destroy(void);
	};
}