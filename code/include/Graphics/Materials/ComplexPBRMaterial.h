#pragma once
#include "ComplexMaterial.h"
#include "PBRMaterial.h"

namespace Pu
{
	/* Defines a complex physically based material without texture maps. */
	class ComplexPBRMaterial
		: public PBRMaterial, ComplexMaterial
	{
	public:
		/* Initializes a new instance of a complex physically based material. */
		ComplexPBRMaterial(_In_ const string &name)
			: PBRMaterial(name), ComplexMaterial()
		{}

		/* Copy constructor. */
		ComplexPBRMaterial(_In_ const ComplexPBRMaterial &value)
			: PBRMaterial(value), ComplexMaterial(value)
		{}

		/* Move constructor. */
		ComplexPBRMaterial(_In_ ComplexPBRMaterial &&value)
			: PBRMaterial(std::move(value)), ComplexMaterial(std::move(value))
		{}

		/* Releases the resources allocated by the material. */
		virtual ~ComplexPBRMaterial(void) {}

		/* Copy assignment. */
		_Check_return_ inline ComplexPBRMaterial& operator =(_In_ const ComplexPBRMaterial &other)
		{
			if (this != &other)
			{
				PBRMaterial::operator=(other);
				ComplexMaterial::operator=(other);
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline ComplexPBRMaterial& operator =(_In_ ComplexPBRMaterial &&other)
		{
			if (this != &other)
			{
				PBRMaterial::operator=(std::move(other));
				ComplexMaterial::operator=(std::move(other));
			}

			return *this;
		}

	protected:
		/* Gets a unique indentifier of the material type (used for sorting). */
		_Check_return_ virtual inline size_t GetMaterialType(void) const override
		{
			return typeid(ComplexPBRMaterial).hash_code();
		}
	};
}