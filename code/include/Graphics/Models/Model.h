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

		/* Gets the meshes in this model. */
		_Check_return_ inline const vector<Shape>& GetMeshes(void) const
		{
			return Meshes;
		}

		/* Gets the material at the specified index. */
		_Check_return_ inline const Material& GetMaterial(_In_ uint32 idx) const
		{
			return Materials.at(idx);
		}

		/* Gets the material used to render the light probes at the specified index. */
		_Check_return_ inline const DescriptorSet& GetProbeMaterial(_In_ uint32 idx) const
		{
			return ProbeMaterials.at(idx);
		}

	protected:
		/* Defines the meshes and their material indices. */
		vector<Shape> Meshes;
		/* Defines the materials. */
		vector<Material> Materials;
		/* Defines the materials used in the light probes. */
		vector<DescriptorSet> ProbeMaterials;

		/* Increases the reference counter by one and returns itself. */
		_Check_return_ virtual Asset& Duplicate(_In_ AssetCache&);

	private:
		friend class AssetLoader;
		friend class AssetFetcher;

		DescriptorPool *poolMaterials, *poolProbes;
		Buffer *gpuData;
		vector<Texture2D*> textures;

		void Initialize(LogicalDevice &device, const PuMData &data);
		void Finalize(CommandBuffer &cmdBuffer, const DeferredRenderer &deferred, const LightProbeRenderer &probes, const PuMData &data);
		void Destroy(void);
	};
}