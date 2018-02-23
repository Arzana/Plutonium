#pragma once
#include "GameLogic\WorldObject.h"
#include "Shape.h"

namespace Plutonium
{
	/* Defines a basic multitextured model that can be placed in the world. */
	struct Model
		: public WorldObject
	{
		Model(_In_ const Model &value) = delete;
		Model(_In_ Model &&value) = delete;
		/* Releases the resources allocated by the model. */
		~Model(void);

		_Check_return_ Model& operator =(_In_ const Model &other) = delete;
		_Check_return_ Model& operator =(_In_ Model &&other) = delete;

		/* Loads a model from a specified .obj file (requires delete!). */
		_Check_return_ static Model* FromFile(_In_ const char *path);

	private:
		friend struct Renderer;

		Model(void)
			: WorldObject()
		{}

		std::vector<Shape*> shapes;
	};
}