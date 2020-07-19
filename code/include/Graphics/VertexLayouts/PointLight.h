#pragma once
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines the vertex layout used by the instanced point lights. */
	struct PointLight
	{
	public:
		/* Defines the volume transformation of the light. */
		Matrix Volume;
		/* Defines the color of the light. */
		Vector3 Radiance;
		/* Defines the constant factor for the light attenuation. */
		float AttenuationC;
		/* Defines the linear factor for the light attenuation. */
		float AttenuationL;
		/* Defines the quadratic factor for the light attenuation. */
		float AttenuationQ;
	};
}