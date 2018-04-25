#pragma once
#include "Light.h"

namespace Plutonium
{
	/* Defines a basic point light source. */
	struct PointLight
		: Light
	{
		/* Defines the world position of the light source. */
		Vector3 Position;
		/* Defines the constant attenuation value. */
		float Constant;
		/* Defines the linear attenuation value. */
		float Linear;
		/* Defines the quadratic attenuation value. */
		float Quadratic;

		/* Initializes a new instance of a point light source. */
		PointLight(_In_ Vector3 position, _In_ Color color, _In_ float constant, _In_ float linear, _In_ float quadratic)
			: Light(color, color, color), Position(position),
			Constant(constant), Linear(linear), Quadratic(quadratic)
		{}

		PointLight(_In_ const PointLight &value) = delete;
		PointLight(_In_ PointLight &&value) = delete;

		_Check_return_ PointLight& operator =(_In_ const PointLight &other) = delete;
		_Check_return_ PointLight& operator =(_In_ PointLight &&other) = delete;

		/* Sets the attenuation terms to specified values. */
		inline void SetAttenuation(_In_ float constant, _In_ float linear, _In_ float quadratic)
		{
			Constant = constant;
			Linear = linear;
			Quadratic = quadratic;
		}
	};
}