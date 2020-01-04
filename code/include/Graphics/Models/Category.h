#pragma once

namespace Pu
{
	/* Defines the category of a specific model. */
	enum class ModelCategory
	{
		/* Defines simple static geomerty such as a skybox and terrain. */
		Environment,
		/* Defines (typically large) immovable objects. */
		Stationary,
		/* Defines objects without animation, but that might move through the scene. */
		Static,
		/* Defines objects with animation. */
		Dynamic,
		/* Defines additional effects such as particles and coats. */
		Effect
	};
}