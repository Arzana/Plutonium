#pragma once
#include "ComplexMaterial.h"
#include "Graphics/Color.h"

namespace Pu
{
	/* Defines the basic properties of a physically based rendering material. */
	class PBRMaterial
		: public Material
	{
	public:
		/* Defines the refracted color of the material (defaults to white). */
		Color DiffuseFactor;
		/* Defines the material's surface roughness (defaults to smooth). */
		float Glossiness;
		/* Defines the reflected color of the material (defaults to white). */
		Color SpecularFactor;

		/* Initializes a new instance of a physically based rendering material, with default parameters. */
		PBRMaterial(_In_ const string &name)
			: Material(name), DiffuseFactor(Color::White()), Glossiness(1.0f), SpecularFactor(Color::White())
		{}

		/* Copy constructor. */
		PBRMaterial(_In_ const PBRMaterial &value)
			: Material(value), DiffuseFactor(value.DiffuseFactor), Glossiness(value.Glossiness), SpecularFactor(value.SpecularFactor)
		{}

		/* Move constructor. */
		PBRMaterial(_In_ PBRMaterial &&value)
			: Material(std::move(value)), DiffuseFactor(value.DiffuseFactor), Glossiness(value.Glossiness), SpecularFactor(value.SpecularFactor)
		{}

		/* Releases the resources allocated by the material. */
		virtual ~PBRMaterial(void) {}

		/* Copy assignment. */
		_Check_return_ inline PBRMaterial& operator =(_In_ const PBRMaterial &other)
		{
			if (this != &other)
			{
				Material::operator=(other);
				DiffuseFactor = other.DiffuseFactor;
				Glossiness = other.Glossiness;
				SpecularFactor = other.SpecularFactor;
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline PBRMaterial& operator =(_In_ PBRMaterial &&other)
		{
			if (this != &other)
			{
				Material::operator=(std::move(other));
				DiffuseFactor = other.DiffuseFactor;
				Glossiness = other.Glossiness;
				SpecularFactor = other.SpecularFactor;
			}

			return *this;
		}

	protected:
		/* Gets a unique indentifier of the material type (used for sorting). */
		_Check_return_ virtual inline size_t GetMaterialType(void) const override
		{
			return typeid(PBRMaterial).hash_code();
		}
	};
}