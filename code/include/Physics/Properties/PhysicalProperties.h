#pragma once
#include "MechanicalProperties.h"

namespace Pu
{
	/* Defines all of the physical properties of a specific material. */
	struct PhysicalProperties
	{
		/* Specifies the density (in kilograms per cubic meter) of the material. */
		float Density;
		/* Specifies the mechanical properties of the material. */
		MechanicalProperties Mechanical;
	};
}