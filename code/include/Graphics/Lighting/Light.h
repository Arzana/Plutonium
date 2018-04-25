#pragma once
#include "Graphics\Color.h"

namespace Plutonium
{
	/* Defines the base class for a light source. */
	struct Light
	{
	public:
		/* The ambient color of the light. */
		Color Ambient;
		/* The diffuse color of the light. */
		Color Diffuse;
		/* The specular color of the light. */
		Color Specular;

		Light(_In_ const Light &value) = delete;
		Light(_In_ Light &&value) = delete;

		_Check_return_ Light& operator =(_In_ const Light &other) = delete;
		_Check_return_ Light& operator =(_In_ Light &&other) = delete;

		/* Sets the ambient, diffuse and specular color to the specified value. */
		inline void SetLightColor(_In_ Color color)
		{
			Ambient = color;
			Diffuse = color;
			Specular = color;
		}

	protected:
		Light(Color ambient = Color::Black, Color diffuse = Color::Black, Color specular = Color::Black)
			: Ambient(ambient), Diffuse(diffuse), Specular(specular)
		{}
	};
}