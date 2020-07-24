#pragma once

namespace Pu
{
	/* Defines the mechanical properties of a physical object. */
	struct MechanicalProperties
	{
		/* Specifies the coefficient of restitution [0, 1]. */
		float CoR;
		/* Specifies the coefficient of static friction [0, n]. */
		float CoFs;
		/* Specifies the coefficient of kinetic friction [0, n]. */
		float CoFk;
		/* Specifies the coefficient of rolling fricion [0, n]. */
		float CoFr;
	};
}