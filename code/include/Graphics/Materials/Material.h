#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Defines a base class for all material types. */
	class Material
	{
	public:
		/* The name of this material. */
		string Name;
		/* Whether meshes with this material should be rendered. */
		bool Visible;

		/* Initializes a new instance of a material. */
		Material(_In_ const string &name)
			: Name(name), Visible(true)
		{}

		/* Copy constructor. */
		Material(_In_ const Material &value)
			: Name(value.Name), Visible(value.Visible)
		{}

		/* Move constructor. */
		Material(_In_ Material &&value)
			: Name(std::move(value.Name)), Visible(value.Visible)
		{}

		/* Releases the resources allocated by the material. */
		virtual ~Material(void) {}

		/* Copy assignment. */
		_Check_return_ inline Material& operator =(_In_ const Material &other)
		{
			if (this != &other)
			{
				Name = other.Name;
				Visible = other.Visible;
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline Material& operator =(_In_ Material &&other)
		{
			if (this != &other)
			{
				Name = std::move(other.Name);
				Visible = other.Visible;
			}

			return *this;
		}

	protected:
		/* Gets a unique indentifier of the material type (used for sorting). */
		_Check_return_ virtual inline size_t GetMaterialType(void) const
		{
			return typeid(Material).hash_code();
		}
	};
}