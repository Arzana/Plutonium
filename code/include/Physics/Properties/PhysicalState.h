#pragma once

namespace Pu
{
	/* Defines the current state of an object's physical properties. */
	struct PhysicalState
	{
		/* Specifies the volume (in cubic meters) of the physical object. */
		float Volume;
		/* Specifies the mass (in kilograms) of the physical object. */
		float Mass;
		/* Specifies the coefficient of drag of the shape of the physical object. */
		float Cd;
	};
}