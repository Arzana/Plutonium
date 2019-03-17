#pragma once
#include "Material.h"

namespace Pu
{
	/* Defines the types of alpha blending available. */
	enum class AlphaMode
	{
		/* No blending, alpha channel is ignored. */
		Opaque,
		/* Fully opaque if below alpha cutoff, otherwise; fully transparent. */
		Toggle,
		/* Fully blended using Porter and Duff depending on alpha channel. */
		Blended
	};

	/* Defines an extension material for more complex materials that use blending and culling. */
	class ComplexMaterial
	{
	public:
		/* Defines how the material should deal with opacity. */
		AlphaMode AlphaMode;
		/* Defines the cutoff of materials with Toggle alpha mode. */
		float AlphaCutoff;
		/* Defines whether back-face culling should be disabled. */
		bool DoubleSided;

		/* Initializes a new instance of a complex material as a one-sided opaque material. */
		ComplexMaterial(void)
			: AlphaMode(AlphaMode::Opaque), AlphaCutoff(0.0f), DoubleSided(false)
		{}

		/* Copy constructor. */
		ComplexMaterial(_In_ const ComplexMaterial &value)
			: AlphaMode(value.AlphaMode), AlphaCutoff(value.AlphaCutoff), DoubleSided(value.DoubleSided)
		{}

		/* Move constructor. */
		ComplexMaterial(_In_ ComplexMaterial &&value)
			: AlphaMode(value.AlphaMode), AlphaCutoff(value.AlphaCutoff), DoubleSided(value.DoubleSided)
		{}

		/* Releases the resources allocated by the material. */
		virtual ~ComplexMaterial(void) {}

		/* Copy assignment. */
		_Check_return_ inline ComplexMaterial& operator =(_In_ const ComplexMaterial &other)
		{
			if (this != &other)
			{
				AlphaMode = other.AlphaMode;
				AlphaCutoff = other.AlphaCutoff;
				DoubleSided = other.DoubleSided;
			}

			return *this;
		}

		/* Move assignment. */
		_Check_return_ inline ComplexMaterial& operator =(_In_ ComplexMaterial &&other)
		{
			if (this != &other)
			{
				AlphaMode = other.AlphaMode;
				AlphaCutoff = other.AlphaCutoff;
				DoubleSided = other.DoubleSided;
			}

			return *this;
		}
	};
}