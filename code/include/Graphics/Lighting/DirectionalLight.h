#pragma once
#include "Light.h"

namespace Plutonium
{
	/* Defines a basic directional light source. */
	struct DirectionalLight
		: Light
	{
		/* The direction of the light source. */
		Vector3 Direction;

		/* Initializes a new instance of a directional light source. */
		DirectionalLight(_In_ Vector3 direction, _In_ Color ambient, _In_ Color diffuse, _In_ Color specular)
			: Light(ambient, diffuse, specular), Direction(direction)
		{}

		DirectionalLight(_In_ const DirectionalLight &value) = delete;
		DirectionalLight(_In_ DirectionalLight &&value) = delete;

		_Check_return_ DirectionalLight& operator =(_In_ const DirectionalLight &other) = delete;
		_Check_return_ DirectionalLight& operator =(_In_ DirectionalLight &&other) = delete;
	};
}