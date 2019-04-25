#pragma once
#include "PBRMaterial.h"
#include "Graphics/Textures/Texture2D.h"

namespace Pu
{
	/* Defines all properties of a physically based rendering material. */
	class MappedPBRMaterial
		: public PBRMaterial
	{
	public:
		/* Defines the refracted color of the material at every pixel. */
		const Texture2D &DiffuseTexture;
		/* Defines the reflected color and the roughness of the surface at every pixel. */
		const Texture2D &SpecularGlossinessTexture;

		/* Initializes a new instance of a mapped PBR material. */
		MappedPBRMaterial(_In_ const string &name, _In_ const Texture2D &diffuse, _In_ const Texture2D &specGloss)
			: PBRMaterial(name), DiffuseTexture(diffuse), SpecularGlossinessTexture(specGloss)
		{}

		MappedPBRMaterial(_In_ const MappedPBRMaterial&) = delete;

		/* Move constructor. */
		MappedPBRMaterial(_In_ MappedPBRMaterial &&value)
			: PBRMaterial(std::move(value)), DiffuseTexture(std::move(value.DiffuseTexture)),
			SpecularGlossinessTexture(std::move(value.SpecularGlossinessTexture))
		{}

		_Check_return_ MappedPBRMaterial& operator =(_In_ const MappedPBRMaterial&) = delete;
		_Check_return_ MappedPBRMaterial& operator =(_In_ MappedPBRMaterial&&) = delete;

	protected:
		/* Gets a unique indentifier of the material type (used for sorting). */
		_Check_return_ virtual inline size_t GetMaterialType(void) const override
		{
			return typeid(MappedPBRMaterial).hash_code();
		}
	};
}