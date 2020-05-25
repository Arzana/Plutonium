#pragma once

namespace Pu
{
	/* Defines the mechanical properties of a physical object. */
	struct MechanicalProperties
	{
		/* Specifies the coefficient of restitution [0, 1]. */
		float CoR;
		/* Specifies the coefficient of friction [0, n]. */
		float CoF;
	};
}